#ifndef REFLMEMPP_H
#define REFLMEMPP_H

#include "detail.h"
#include "utils.h"
#include <experimental/meta>
#include <iostream>
#include <memory>
#include <memory_resource>
#include <span>
#include <type_traits>

using namespace std::literals::string_view_literals;

namespace rmpp {
constexpr auto member_prefix = ""sv;

static size_t max_inner_array_elem = 256; // max elements for inner array per data element

template <typename T>
struct ValHelper {
  struct SVal;
};

template <typename S, size_t Alignment>
class AoS2SoA {
  static_assert(type_is_struct(^^S) && nsdms(^^S).size() > 0,
                "SoA can only be created for structs with data members");

private:
  std::span<std::byte> storage;
  size_t m_size;      // Number of data elements currently stored in the SoA
  size_t m_capacity;  // Maximum number of data elements that can be stored in the SoA

  constexpr static inline size_t align_size(size_t size) { return ((size + Alignment - 1) / Alignment) * Alignment; }

  /// -------------------------------------------------------------------------------------
  /// Compute the size in bytes for a SoA member. If the member is a
  /// struct, the size is computed recursively by summing the sizes of the
  /// scalar submembers.
  /// -------
  /// Params:
  /// -------
  /// Member: member to compute the size for
  /// size: in bytes
  template <std::meta::info Member>
  constexpr static size_t compute_size(size_t n) {
    constexpr auto id = identifier_of(Member);
    constexpr auto type = type_remove_cvref(type_of(Member));

    size_t size = 0;

    if constexpr (type_is_container(type)) {
      constexpr auto value_type = get_scalar_type(type);
      size = align_size(max_inner_array_elem * sizeof(typename[:value_type:]));
    } else if constexpr (type_is_struct(type) && nsdms(type).size() > 0) {
      size = [:expand_all(nsdms(type)):] >> [&]<auto... submembers> {
        return (0 + ... + compute_size<submembers>(n));
      };
    } else {
      size = align_size(n * sizeof(typename[:type_of(Member):]));
    }

#ifdef RMPP_DEBUG
    std::cout << "Size of SoA member " << id << " is " << size << " bytes\n";
#endif
    return size;
  }

  /// -------------------------------------------------------------------------------------
  /// Generate a view of a given AoS element.
  /// -------
  /// Params:
  /// -------
  /// ViewType: type of the view (e.g., AoS or AoS const)
  /// idx: index of the element in the AoS
  /// Return: view of the element
  template <typename ViewType>
  ViewType generate_view(const size_t idx) const {
    consteval {
      std::meta::list_builder member_data_tokens{};
      for (auto member : nsdms(^^S)) {
        member_data_tokens += detail::views::assign_aos_view_member(member, ^^idx, ^^{ \id(identifier_of(member))});
      }

      __report_and_inject(^^{ return ViewType{\tokens(member_data_tokens)}; });
    }
  }

public:
  consteval { detail::gen_soa_members(^^S, true); }

  using aos_view = S;
  using aos_cview = const S;

  /// -------------------------------------------------------------------------------------
  /// Generate the value type version of the proxy struct S.

  // // Crashes due to a compiler bug if there are std::vector members.
  // struct SVal;
  // consteval {
  //   __report_and_inject(^^{
  //     struct SVal { \tokens(detail::gen_value_type(^^S)) };
  //   });
  // }

  // Workaround:
  consteval {
    namespace_inject(^^rmpp, ^^{
      template <>
      struct ValHelper<typename[: \(^^S):]>::SVal {
        \tokens(detail::gen_value_type(^^S))
      };
    });
  }
  using value_type = ValHelper<S>::SVal;

  /// -------------------------------------------------------------------------------------
  /// Computes the total size needed for the storage buffer in bytes, to store n data
  /// elements in each SoA member.
  static size_t ComputeSize(size_t n) {
    return [:expand_all(nsdms(^^S)):] >> [n]<auto... M> { return (0 + ... + compute_size<M>(n)); };
  }

  /// -------------------------------------------------------------------------------------
  /// Constructor for that takes the number of data elements. Uses `max_inner_array_elem`
  /// as the size allocated to inner array elements for all data elements in total.
  /// -------
  /// Params:
  /// -------
  /// buf: pointer to the storage buffer
  /// buf_size: size of the storage buffer in bytes
  /// capacity: number of data elements that can be stored in the SoA
  AoS2SoA(std::byte *buf, const size_t buf_size, const size_t capacity) {
#ifdef RMPP_DEBUG
    std::cout << "storage at " << (long long)static_cast<void *>(buf) << " with capacity " << capacity << " and size "
              << buf_size << " bytes\n";
#endif

    storage = std::span<std::byte>(buf, buf_size);
    m_size = 0;
    m_capacity = capacity;

    // Initialize the spans for each member in the SoA to offsets into
    // the storage container.
    consteval {
      std::meta::info offset_tokens = ^^{ 0 };
      for (auto member : nsdms(^^S)) {
        offset_tokens = detail::gen_soa_init(member, ^^{ \id(identifier_of(member))}, offset_tokens);
      }
    }
  }

  size_t size() const { return m_size; }

  aos_view operator[](const size_t idx) { return generate_view<aos_view>(idx); }

  aos_cview operator[](const size_t idx) const { return generate_view<aos_cview>(idx); }

  void push_back() { m_size++; }

  void push_back(const value_type obj) {
    if (m_size == m_capacity) {
      // todo: reallocate?
      std::cerr << "Capacity reached" << std::endl;
      return;
    }

    consteval {
      for (auto member : nsdms(^^S)) {
        detail::gen_push_back(member, ^^{ \id(identifier_of(member))});
      }
    }

    m_size++;
  }
};
} // namespace rmpp
#endif
