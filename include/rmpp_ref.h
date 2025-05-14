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
