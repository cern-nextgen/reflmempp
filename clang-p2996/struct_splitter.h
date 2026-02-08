// Compile with clang-p2996
#ifndef STRUCT_SPLITTER_H
#define STRUCT_SPLITTER_H

#include <array>
#include <cassert>
#include <experimental/meta>
#include <iostream>
#include <new>
#include <ranges>

////////////// Reflection utilities

template <auto... V>
struct replicator_type {
  template <typename F>
  constexpr auto operator>>(F body) const -> decltype(auto) {
    return body.template operator()<V...>();
  }
};

template <auto... V>
replicator_type<V...> replicator = {};

consteval auto ExpandAll(std::span<std::meta::info const> r) -> std::meta::info {
  std::vector<std::meta::info> rv;
  for (std::meta::info i : r) {
    rv.push_back(reflect_constant(i));
  }
  return substitute(^^replicator, rv);
}

consteval auto nsdms(std::meta::info type) {
  return define_static_array(nonstatic_data_members_of(type, std::meta::access_context::current()));
}

//////////////

consteval auto SplitOp(std::vector<int> indices) {
  return define_static_array(indices);
}

/* Helper method to get a list of data_member_specs for members in
 * the struct S based on the given a list of indices
 */
template <typename S>
consteval auto GetMemberSpecs(std::span<const int> indices) {
  auto members = nonstatic_data_members_of(^^S, std::meta::access_context::unchecked());

  std::vector<std::meta::info> specs;
  for (auto index : indices) {
    auto mem_descr = data_member_spec(remove_cvref(type_of(members[index])), {.name = identifier_of(members[index])});
    specs.push_back(mem_descr);
  }
  return specs;
}

// clang-format off
/* Takes a struct In and generate a template specialization of struct Out
 * for each SplitOp provided, containing only the members of struct In, in the
 * order specified by the SplitOp.
 */
template <typename In, template <auto> typename Out, typename... SplitOps>
consteval void SplitStruct(SplitOps... ops) {
  (define_aggregate(substitute(^^Out, { std::meta::reflect_constant_array(ops)}),
                    GetMemberSpecs<In>(ops)),
   ...);
};
// clang-format on

/* Compute the aligned size for a given size and alignment. */
inline size_t AlignSize(size_t size, size_t alignment) {
  return (size + alignment - 1) / alignment * alignment;
}

///////////////////

/* Get the mapping of the original members to their locations in the partitions.
 * Returns a static array of pairs, where each pair contains the partition index
 * and the member index within that partition.
 */
#pragma clang diagnostic ignored "-Wreturn-type"
consteval auto find_in_partitions(std::meta::info original_type, std::meta::info partitioned_type) {
  auto original_members = nsdms(original_type);
  auto partitions = nsdms(partitioned_type);
  std::vector<std::pair<size_t, size_t>> mapping(original_members.size());

  for (size_t ip = 0; ip < partitions.size(); ++ip) {
    auto partition_type = is_pointer_type(type_of(partitions[ip])) ? remove_pointer(type_of(partitions[ip]))
                                                                   : template_arguments_of(type_of(partitions[ip]))[0];
    auto partition_members = nsdms(partition_type);
    for (size_t im = 0; im < partition_members.size(); ++im) {
      for (size_t io = 0; io < original_members.size(); ++io) {
        if (identifier_of(original_members[io]) == identifier_of(partition_members[im])) {
          mapping[io] = {ip, im};
          break;
        }
      }
    }
  }

  return std::define_static_array(mapping);
}

/// A container that takes substructures with a subset of the members of ProxyRef in its
/// template parameters T..., and stores each substructure in a separate array.
/// The substructure arrays are stored contiguously in memory, aligned to the size of a cacheline.
template <typename ProxyRef, typename... T>
struct PartitionedContainerContiguous {
private:
  static_assert(nsdms(^^ProxyRef).size() == (... + nsdms(^^T).size()),
                "PartitionedContainerContiguous: Total number of members in substructures"
                "must equal number of members in ProxyRef");
  struct Partitions;
  consteval { define_aggregate(^^Partitions, {data_member_spec(substitute(^^std::span, {^^T}))...}); }

  Partitions p;
  std::byte *storage;
  size_t n;

  static constexpr auto mapping = find_in_partitions(^^ProxyRef, ^^Partitions);

public:
  PartitionedContainerContiguous(size_t n, size_t alignment) : n(n) {
    // Allocate each partition
    size_t total_size = (0 + ... + AlignSize(n * sizeof(T), alignment));
    storage = static_cast<std::byte *>(std::aligned_alloc(alignment, total_size));

    // Assign each partition to its location in the storage vector
    size_t offset = 0;
    template for (constexpr auto I : std::views::iota(0zu, sizeof...(T))) {
      using MemType = T...[I];
      constexpr auto m = nsdms(^^Partitions)[I];

      // https://learn.microsoft.com/en-us/cpp/standard-library/new-operators?view=msvc-170#op_new_arr
      p.[:m:] = std::span<MemType>(std::launder(reinterpret_cast<MemType *>(new (&storage[offset]) MemType[n])), n);
      offset += AlignSize(p.[:m:].size_bytes(), alignment);
    }
  }

  inline ProxyRef operator[](const size_t index) const {
    return [&]<size_t... Is>(std::index_sequence<Is...>) constexpr -> ProxyRef {
      constexpr auto partitions = nsdms(^^Partitions);
      return ProxyRef{
          p.[:partitions[mapping[Is].first]:][index].[:nsdms(template_arguments_of(type_of(
                                                           partitions[mapping[Is].first]))[0])[mapping[Is].second
      ]:]...};
    }(std::make_index_sequence<nsdms(^^ProxyRef).size()>());
  }

  size_t size() const { return n; }

  static std::string to_string() { return std::string("PartitionedContainerContiguous") + GetSplitOpsString<T...>(); }

  ~PartitionedContainerContiguous() {
    // Deallocate each partition
    template for (constexpr auto I : std::views::iota(0zu, sizeof...(T))) {
      using MemType = T...[I];
      constexpr auto m = nsdms(^^Partitions)[I];

      // Need to deallocate each element individually because it is constructed using placement new.
      for (size_t i = n - 1; i == 0; --i) {
        p.[:m:][i].~MemType();
      }
    }

    std::free(storage);
  }
};

/// A container that takes substructures with a subset of the members of ProxyRef in its
/// template parameters T..., and stores each substructure in a separate array.
/// The substructures are stored in separate arrays aligned to cacheline size,
/// in different memory locations.
template <typename ProxyRef, typename... T>
struct PartitionedContainer {
private:
  static_assert(nsdms(^^ProxyRef).size() == (... + nsdms(^^T).size()),
                "PartitionedContainer: Total number of members in partitions "
                "must equal number of members in ProxyRef");
  struct Partitions;
  consteval { define_aggregate(^^Partitions, {data_member_spec(add_pointer(^^T))...}); }

  Partitions p;
  size_t n;

  static constexpr auto mapping = find_in_partitions(^^ProxyRef, ^^Partitions);

public:
  PartitionedContainer(size_t n, size_t alignment) : n(n) {
    // Allocate each partition
    template for (constexpr auto m : nsdms(^^Partitions)) {
      p.[:m:] = static_cast<typename[:type_of(m):]>(std::aligned_alloc(
                  alignment, AlignSize(n * sizeof(typename[:remove_pointer(type_of(m)):]), alignment)));
    }
  }

  inline ProxyRef operator[](const size_t index) const {
    return [&]<size_t... Is>(std::index_sequence<Is...>) constexpr -> ProxyRef {
      constexpr auto partitions = nsdms(^^Partitions);
      return ProxyRef{
          p.[:partitions[mapping[Is].first]:][index].[:nsdms(remove_pointer(type_of(
                                                           partitions[mapping[Is].first])))[mapping[Is].second]:]...};
    }(std::make_index_sequence<nsdms(^^ProxyRef).size()>());
  }

  size_t size() const { return n; }

  static std::string to_string() { return std::string("PartitionedContainer") + GetSplitOpsString<T...>(); }

  ~PartitionedContainer() {
    // Deallocate each partition
    template for (constexpr auto m : nsdms(^^Partitions)) {
      std::free(p.[:m:]);
    }
  }
};

#endif // STRUCT_SPLITTER_H