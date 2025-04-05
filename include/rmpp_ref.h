#ifndef REFLMEMPP_H
#define REFLMEMPP_H

#include "utils.h"
#include <experimental/meta>
#include <iostream>
#include <memory>
#include <memory_resource>
#include <span>
#include <type_traits>

namespace rmpp {
using namespace std::literals::string_view_literals;

constexpr auto member_prefix = ""sv;

namespace detail {

/// @brief Check if a given reflection is in a vector of reflections.
/// @param vec vectro of reflections
/// @param refl reflection to check
/// @return true if the reflection is in the vector, false otherwise
consteval bool contains_refl(std::vector<std::meta::info> vec, std::meta::info refl) {
  for (auto r : vec) {
    if (r == refl) return true;
  }
  return false;
}

/// @brief Injects declarations for a Structure of Arrays (SoA) version of the AoS
/// members. If a member is a struct, it recursively generates a nested struct
/// containing the span versions of the submembers. The nested struct has the
/// same name as the original struct with "SoA" appended.
/// @param S Struct used for AoS
/// @param inject If true, inject the declarations into the current scope
/// @return Tokens for declarations of SoA members in the current scope.
consteval std::meta::info gen_soa_members(std::meta::info S, bool inject) {
  // List of tokens for SoA member declarations.
  std::vector<std::meta::info> decl_tokens;

  // List of already declared structs in the current scope.
  std::vector<std::meta::info> visited_structs;

  for (auto member : nonstatic_data_members_of(S)) {
    auto type = type_remove_cvref(type_of(member));
    auto name = identifier_of(member);

    if (type_is_struct(type)) {
      // Get tokens for declaring a version of the nested struct
      // with span members. Skip the declaration if the struct
      // has already been declared in the current scope.
      if (!contains_refl(visited_structs, type)) {
        visited_structs.push_back(type);

        // Build declaration of nested struct.
        auto subdecl_tokens = gen_soa_members(type, false);
        decl_tokens.push_back(^^{ struct \id(identifier_of(type), "SoA"sv){
                                \tokens(subdecl_tokens)}; });
      }

      decl_tokens.push_back(^^{ \id(identifier_of(type), "SoA"sv) \id(name); });

    } else { // Basic data member.
      decl_tokens.push_back(^^{ std::span<typename[:\(type):]> \id(name); });
    }
  }

  // Concatenate the declarations into a single token string,
  // separated by semicolons.
  std::meta::info decl_concat = ^^{};
  for (auto tks : decl_tokens) {
    decl_concat = ^^{ \tokens(decl_concat)
            \tokens(tks) };
  }

  // Inject the declarations.
  // Only inject if this is the top-level struct, otherwise we return
  // the tokens, so they can be wrapped inside the declaration of the
  // parent struct.
  if (inject) {
    __report_tokens(decl_concat);
    queue_injection(decl_concat);
  }

  return decl_concat;
}

/// @brief Generate initalization of a given SoA member.
/// @param member reflection of the corresponding AoS member
/// @param id_tokens tokens for the identifier of the current member
/// @param offset_tokens tokens for the offset into the storage for the current
/// span
/// @return current total offset into the storage
consteval std::meta::info gen_soa_init(std::meta::info member, std::meta::info id_tokens,
                                       std::meta::info offset_tokens) {
  auto id = identifier_of(member);
  auto type = type_remove_cvref(type_of(member));

  if (type_is_struct(type)) {
    // Recursively initialize nested struct members. Add the identifier of
    // the current member to the id_tokens to access the members of the
    // nested struct.
    for (auto submember : nonstatic_data_members_of(type)) {
      auto offset = gen_soa_init(submember, ^^{
                \tokens(id_tokens).\id(identifier_of(submember))}, offset_tokens);
      offset_tokens = ^^{ \tokens(offset_tokens) + \tokens(offset) };
    }
  } else { // Scalar members
    // __report_tokens(offset_tokens);

    // Inject span initialization for the current member.
    queue_injection(^^{
      \tokens(id_tokens) =
          std::span(reinterpret_cast<typename[: \(type):] *>(storage.data() + \tokens(offset_tokens)), m_size);
    });

    auto member_size = ^^{ align_size(m_size * sizeof(typename[: \(type):])) };
    auto next_offset_tokens = ^^{ \tokens(offset_tokens) + \tokens(member_size) };
    queue_injection(^^{
      for (size_t i = \tokens(offset_tokens); i < \tokens(next_offset_tokens); i += sizeof(typename[: \(type):])) {
        std::construct_at(reinterpret_cast<typename[: \(type):] *>(storage.data() + i), typename[: \(type):]());
      }
    });

    // Tokens for comuting the offset into the storage for the next
    // member.
    offset_tokens = next_offset_tokens;
  }

  return offset_tokens;
}

namespace views {

/// @brief Get the tokens needed to initialize the view of a given AoS member.
/// @param member member to generate view for
/// @param idx idx of the current element in the AoS
/// @param id_tokens tokens for the identifier of the current (nested) member
/// @return tokens for initializing the view of the current member
consteval std::meta::info assign_aos_view_member(std::meta::info member, std::meta::info idx,
                                                 std::meta::info id_tokens) {
  auto name = identifier_of(member);
  auto type = type_of(member);

  if (type_is_struct(type)) {
    // Recursively initialize nested struct members. Add the identifier of
    // the current member to the id_tokens to access the members of the
    // nested struct.
    std::meta::list_builder substruct_assign{};
    for (auto submember : nonstatic_data_members_of(type)) {
      auto submember_id = ^^{ \tokens(id_tokens).\id(identifier_of(submember)) };
      substruct_assign += assign_aos_view_member(submember, idx, submember_id);
    }

    // Nested struct members. e.g., .vecs = { .a = a[idx], .b = b[idx] }
    return ^^{
      { \tokens(substruct_assign) }
    };
  }

  // Scalar members. e.g., .x = x[idx]
  return ^^{ \tokens(id_tokens)[\id(identifier_of(idx))] };
}

} // namespace views
} // namespace detail

template <typename S, size_t Alignment>
class vector {
  static_assert(type_is_struct(^^S));

private:
  alignas(Alignment) std::vector<std::byte> storage;
  size_t m_size;

  struct sov_metadata {
    size_t offset, size;
    friend std::ostream &operator<<(std::ostream &os, const sov_metadata &obj) {
      return os << "{" << obj.offset << ", " << obj.size << "}";
    }
  };

  constexpr inline size_t align_size(size_t size) const { return ((size + Alignment - 1) / Alignment) * Alignment; }

  /// @brief Compute the size in bytes for a SoA member. If the member is a
  /// struct, the size is computed recursively by summing the sizes of the
  /// scalar submembers.
  /// @tparam Member member to compute the size for
  /// @return size in bytes
  template <std::meta::info Member>
  constexpr size_t compute_size() {
    constexpr auto id = identifier_of(Member);
    constexpr auto type = type_remove_cvref(type_of(Member));

    if constexpr (type_is_struct(type)) {
      size_t struct_size = 0;
      [:expand(nonstatic_data_members_of(type)):] >> [&]<auto submember> { struct_size += compute_size<submember>(); };

      return struct_size;
    }

    return align_size(m_size * sizeof(typename[:type_of(Member):]));
  }

  /// @brief Generate a view of a given AoS element.
  /// @tparam ViewType type of the view (e.g., AoS or AoS const)
  /// @param idx index of the element in the AoS
  /// @return view of the element
  template <typename ViewType>
  ViewType generate_view(const size_t idx) const {
    consteval {
      std::meta::list_builder member_data_tokens{};
      for (auto member : nonstatic_data_members_of(^^S)) {
        member_data_tokens += detail::views::assign_aos_view_member(member, ^^idx, ^^{ \id(identifier_of(member))});
      }

      // __report_tokens(member_data_tokens);
      queue_injection(^^{ return ViewType{\tokens(member_data_tokens)}; });
    }
  }

public:
  consteval { detail::gen_soa_members(^^S, true); }

  std::vector<size_t> byte_sizes;

  using aos_view = S;
  using aos_cview = const S;

  /// @brief Constructor for the SoA vector.
  /// @param n number of elements in the AoS.
  vector(const size_t n) {
    m_size = n;

    // Compute the size of the storage container.
    size_t total_byte_size = 0;
    [:expand(nonstatic_data_members_of(^^S)):] >> [&]<auto member> { total_byte_size += compute_size<member>(); };
    std::cout << "storage of " << total_byte_size << " bytes in total\n\n ";
    storage.resize(total_byte_size);

    // Initialze the spans for each member in the SoA to offsets into
    // the storage container.
    consteval {
      std::meta::info offset_tokens = ^^{ 0 };
      for (auto member : nonstatic_data_members_of(^^S)) {
        offset_tokens = detail::gen_soa_init(member, ^^{ \id(identifier_of(member))}, offset_tokens);
      }

      // __report_tokens(offset_tokens);
    }
  }

  size_t size() const { return m_size; }

  aos_view operator[](const size_t idx) { return generate_view<aos_view>(idx); }

  aos_cview operator[](const size_t idx) const { return generate_view<aos_cview>(idx); }
};
} // namespace rmpp
#endif
