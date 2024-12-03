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

constexpr auto member_prefix = "m_"sv;
constexpr auto metadata_suffix = "_md"sv;
constexpr auto extents_suffix = "_extents"sv;

enum class layout { aos, soa };

namespace detail {

/// @brief Generate array members for each non-static data member in a given Structure of Values (SoV) type.
/// @param sov_type SoV type
consteval void gen_soa_members(std::meta::info sov_type) {
  for (auto member : nonstatic_data_members_of(sov_type)) {
    auto vec_member = ^{ \id(member_prefix, name_of(member)) };
    auto type = type_of(member);
    queue_injection(^{ std::span<typename[:\(get_scalar_type(type)):]> \tokens(vec_member); });
  }
}

/// @brief Generate memmbers for a Array of Structures (AoS) view for a given Structure of Values (SoV) type.
/// @param sov_type SoV type
consteval void gen_aos_view(std::meta::info sov_type) {
  for (auto member : nonstatic_data_members_of(sov_type)) {
    auto type = type_of(member);
    queue_injection(^{ typename[:\(type):] & \id(name_of(member)); });
  }
}

/// @brief Generate members for a const Array of Structures (AoS) view for a given Structure of Values (SoV) type.
/// @param sov_type SoV type
consteval void gen_aos_cview(std::meta::info sov_type) {
  for (auto member : nonstatic_data_members_of(sov_type)) {
    auto type = type_of(member);
    queue_injection(^{ const typename[:\(type):] & \id(name_of(member)); });
  }
}

/// @brief Generate members for a Structure of Values version of a Structure of References (SoR) type.
/// @param sor_type SoR type
consteval void gen_sov_members(std::meta::info sor_type) {
  for (auto member : nonstatic_data_members_of(sor_type)) {
    auto type = type_remove_cvref(type_of(member));
    queue_injection(^{ typename[:\(type):] \id(name_of(member)); });
  }
}

} // namespace detail

template <typename S, size_t Alignment, layout L, layout V>
class vector {
private:
  std::vector<std::byte> storage;
  size_t m_size;

  struct value_type {
    consteval { detail::gen_sov_members(^S); }
  };

  struct sov_metadata {
    size_t offset, size;
    friend std::ostream &operator<<(std::ostream &os, const sov_metadata &obj) {
      return os << "{" << obj.offset << ", " << obj.size << "}";
    }
  };

  constexpr inline size_t align_size(size_t size, size_t alignment) const {
    return ((size + alignment - 1) / alignment) * alignment;
  }

  template <std::meta::info Member>
  void compute_sizes_impl(const std::initializer_list<value_type> data, size_t &size, size_t &byte_size) {
    constexpr auto type = type_of(Member);
    if constexpr (type_is_eigen(type)) {
      constexpr auto dim = extract<size_t>(template_arguments_of(type)[1]);
      byte_size = align_size(sizeof(typename[:type_of(Member):][dim * data.size()]), Alignment);
      size = dim * dim * data.size();
    } else if constexpr (type_is_container(type_of(Member))) {
      using vec_type = typename[:type_of(Member):] ::value_type;
      for (auto &elem : data) {
        auto n_elements = elem.[:Member:].size();
        consteval {
          queue_injection(^{
            \id(member_prefix, name_of(Member), metadata_suffix).push_back({.offset = size, .size = n_elements});
          });
        }
        byte_size += align_size(sizeof(vec_type[n_elements]), Alignment);
        size += n_elements;
      }
    } else {
      byte_size = align_size(sizeof(typename[:type_of(Member):][data.size()]), Alignment);
      size = data.size();
    }

    std::cout << "_" << name_of(Member) << " = " << size << " elements in " << byte_size << " bytes\n";
  }

  template <std::meta::info Member>
  void fill_sov_matrix(const std::initializer_list<value_type> data) & {
    using scalar_type = inner_type<typename[:type_of(Member):]>;
    constexpr auto dim = extract<size_t>(template_arguments_of(type_of(Member))[1]);

    size_t e_idx = 0;
    for (size_t i = 0; i < dim; i++) {
      for (size_t j = 0; j < dim; j++) {
        for (auto &elem : data) {
          consteval {
            queue_injection(
                ^{ std::construct_at(&\id(member_prefix, name_of(Member))[e_idx], elem.[:Member:][i][j]); });
          }
          e_idx++;
        }
      }
    }
  }

  template <std::meta::info Member>
  void fill_sov_container(const std::initializer_list<value_type> data) & {
    using vec_type = inner_type<typename[:type_of(Member):]>;

    size_t e_idx = 0;
    for (auto &elem : data) {
      for (size_t i = 0; i < elem.[:Member:].size(); i++) {
        consteval {
          queue_injection(^{ std::construct_at(&\id(member_prefix, name_of(Member))[e_idx], elem.[:Member:][i]); });
        }
        e_idx++;
      }
    }
  }

  template <std::meta::info Member>
  void fill_sov_scalar(const std::initializer_list<value_type> data) & {
    size_t e_idx = 0;
    for (auto &elem : data) {
      consteval {
        queue_injection(^{ std::construct_at(&\id(member_prefix, name_of(Member))[e_idx], elem.[:Member:]); });
      }
      e_idx++;
    }
  }

  template <std::meta::info Member>
  void fill_sov_impl(const std::initializer_list<typename vector::value_type> data) {
    constexpr auto type = type_of(Member);
    if constexpr (type_is_eigen(type)) {
      fill_sov_matrix<Member>(data);
    } else if constexpr (type_is_container(type)) {
      fill_sov_container<Member>(data);
    } else {
      fill_sov_scalar<Member>(data);
    }
  }

  template <typename ViewType>
  ViewType generate_view(const size_t idx) const {
    consteval {
      std::meta::list_builder member_data_tokens{};
      for (auto member : nonstatic_data_members_of(^S)) {
        auto name = name_of(member);
        auto type = type_of(member);
        auto sov_name = ^{ \id(member_prefix, name) };

        if (type_is_eigen(type)) {
          auto dim = extract<size_t>(template_arguments_of(type)[1]);
          auto extents = ^{ typename ViewType::\id(name, extents_suffix) };
          auto stride = ^{
            std::array<size_t, 2> {
              m_size * \(dim), m_size
            }
          };
          member_data_tokens += ^{
            .\id(name) = { \tokens(sov_name).data() + idx, {\tokens(extents){}, \tokens(stride)} }
          };
        } else if (type_is_container(type)) {
          auto md_name = ^{ \id(member_prefix, name, metadata_suffix) };
          member_data_tokens +=
              ^{ .\id(name) = \tokens(sov_name).subspan(\tokens(md_name)[idx].offset, \tokens(md_name)[idx].size) };
        } else {
          member_data_tokens += ^{ .\id(name) = \tokens(sov_name)[idx] };
        }
      }

      __report_tokens(member_data_tokens);
      queue_injection(^{ return ViewType{\tokens(member_data_tokens)}; });
    }
  }

public:
  consteval { detail::gen_soa_members(^value_type); }
  std::vector<size_t> byte_sizes;

  using aos_view = S;

  struct aos_cview {
    consteval { detail::gen_aos_cview(^value_type); }
  };

  vector(std::initializer_list<value_type> data) {
    auto n_members = [:std::meta::reflect_value(nonstatic_data_members_of(^S).size()):];
    m_size = data.size();

    std::vector<size_t> sizes(n_members);
    byte_sizes.resize(n_members);
    size_t total_byte_size = 0;
    size_t m_idx = 0;
    [:expand(nonstatic_data_members_of(^value_type)):] >> [&]<auto member> {
      compute_sizes_impl<member>(data, sizes[m_idx], byte_sizes[m_idx]);
      total_byte_size += byte_sizes[m_idx++];
    };

    storage.resize(total_byte_size);
    std::cout << "storage of " << total_byte_size << " bytes in total\n\n";

    size_t offset = 0;
    m_idx = 0;
    [:expand(nonstatic_data_members_of(^value_type)):] >> [&]<auto member> {
      auto sov_size = sizes[m_idx];

      consteval {
        auto name = name_of(member);
        auto type = get_scalar_type(type_of(member));
        queue_injection(^{
          \id(member_prefix, name) = std::span(reinterpret_cast<[: \(type):] *>(storage.data() + offset), sov_size);
        });
      }
      offset += byte_sizes[m_idx++];
      fill_sov_impl<member>(data);
    };
  }

  size_t size() const { return m_size; }

  aos_view operator[](const size_t idx) { return generate_view<aos_view>(idx); }

  aos_cview operator[](const size_t idx) const { return generate_view<aos_cview>(idx); }

  // vector(const vector &other) : storage(other.storage), m_size(other.m_size), byte_sizes(other.byte_sizes) {
  //   std::cout << "Copy constructor called\n";
  // }

  // vector &operator=(const vector &other) {
  //   if (this != &other) {
  //     storage = other.storage;
  //     m_size = other.m_size;
  //     byte_sizes = other.byte_sizes;
  //     std::cout << "Copy assignment operator called\n";
  //   }
  //   return *this;
  // }
};
} // namespace rmpp
#endif
