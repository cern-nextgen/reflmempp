#ifndef REFL_MEM_PP_H
#define REFL_MEM_PP_H

#include "utils.h"
#include <experimental/meta>
#include <iostream>
#include <memory>
#include <span>
#include <type_traits>

namespace rmpp_ref {
using namespace std::literals::string_view_literals;

constexpr auto member_prefix = "m_"sv;
constexpr auto metadata_suffix = "_md"sv;
constexpr auto extents_suffix = "_extents"sv;

namespace detail {
consteval void gen_sov_members(std::meta::info t) {
  for (auto member : nonstatic_data_members_of(t)) {
    auto vec_member = ^{ \id(member_prefix, name_of(member)) };

    auto type = type_of(member);
    if (type_is_container(type) && !type_is_eigen(type)) {
      queue_injection(^{ std::vector<sov_metadata> \id(member_prefix, name_of(member), metadata_suffix); });
    }

    queue_injection(^{ std::span<typename[:\(get_scalar_type(type)):]> \tokens(vec_member); });
  }
}

consteval void gen_aos_view(std::meta::info t) {
  for (auto member : nonstatic_data_members_of(t)) {
    auto type = type_of(member);
    auto scalar_type = get_scalar_type(type);
    if (type_is_eigen(type)) {
      auto dim = extract<size_t>(template_arguments_of(type)[1]);
      queue_injection(^{ using \id(name_of(member), extents_suffix) = extents<size_t, \(dim), \(dim)>; });
      queue_injection(^{
        mdspan<typename[:\(scalar_type):], \id(name_of(member), extents_suffix), layout_stride> \id(name_of(member));
      });
    } else if (type_is_container(type)) {
      queue_injection(^{ std::span<typename[:\(scalar_type):]>  \id(name_of(member)); });
    } else { // scalar
      queue_injection(^{ typename[:\(type):] & \id(name_of(member)); });
    }
  }
}

consteval void gen_soa_view(std::meta::info t) {
  for (auto member : nonstatic_data_members_of(t)) {
    auto type = type_of(member);
    queue_injection(^{ typename[:\(type):] & \id(name_of(member)); });
  }
}

} // namespace detail

template <typename T, size_t Alignment>
class vector {
private:
  std::vector<std::byte> storage;
  size_t m_size; // Number of elements

public: // internal stuff public for debugging
  struct sov_metadata {
    size_t offset, size;
    friend std::ostream &operator<<(std::ostream &os, const sov_metadata &obj) {
      return os << "{" << obj.offset << ", " << obj.size << "}";
    }
  };
  consteval { detail::gen_sov_members(^T); }

  std::vector<size_t> byte_sizes; // Size of each SoV including alignment padding

  struct aos_view {
    consteval { detail::gen_aos_view(^T); }
  };

  struct soa_view {
    consteval { detail::gen_soa_view(^T); }
  };

  // Helper function to compute aligned size
  constexpr inline size_t align_size(size_t size, size_t alignment) const {
    return ((size + alignment - 1) / alignment) * alignment;
  }

  // Compute the number of bytes needed for each storage vector and the total number of storage bytes.
  template <std::meta::info Member>
  void compute_sizes(const std::initializer_list<T> data, size_t &size, size_t &byte_size) & {
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
  void fill_sov_matrix(const std::initializer_list<T> data) & {
    using scalar_type = inner_type<typename[:type_of(Member):]>;
    constexpr auto dim = extract<size_t>(template_arguments_of(type_of(Member))[1]);

    size_t e_idx = 0;
    // TODO: arbitrary dimensions?
    for (size_t i = 0; i < dim; i++) {
      for (size_t j = 0; j < dim; j++) {
        for (auto &elem : data) {
          consteval {
            // Element-wise contiguous
            // e.g, new (&_m[e_idx]) float(elem.m[i,j]);
            queue_injection(
                ^{ std::construct_at(&\id(member_prefix, name_of(Member))[e_idx], elem.[:Member:][i][j]); });
          }
          e_idx++;
        }
      }
    }
  }

  template <std::meta::info Member>
  void fill_sov_container(const std::initializer_list<T> data) & {
    using vec_type = inner_type<typename[:type_of(Member):]>;

    size_t e_idx = 0;
    for (auto &elem : data) {
      for (size_t i = 0; i < elem.[:Member:].size(); i++) {
        consteval {
          // e.g, new (&_a[e_idx]) int(elem.a[i]);
          queue_injection(^{ std::construct_at(&\id(member_prefix, name_of(Member))[e_idx], elem.[:Member:][i]); });
        }
        e_idx++;
      }
    }
  }

  template <std::meta::info Member>
  void fill_sov_scalar(const std::initializer_list<T> data) & {
    size_t e_idx = 0;
    for (auto &elem : data) {
      consteval {
        // e.g, std::construct_at(&_x[e_idx], elem.x);
        queue_injection(^{ std::construct_at(&\id(member_prefix, name_of(Member))[e_idx], elem.[:Member:]); });
      }
      e_idx++;
    }
  }

public:
  vector(std::initializer_list<T> data) {
    auto n_members = [:std::meta::reflect_value(nonstatic_data_members_of(^T).size()):];
    m_size = data.size();

    std::vector<size_t> sizes(n_members);
    byte_sizes.resize(n_members);
    size_t total_byte_size = 0;
    size_t m_idx = 0;
    [:expand(nonstatic_data_members_of(^T)):] >> [&]<auto member> {
      compute_sizes<member>(data, sizes[m_idx], byte_sizes[m_idx]);
      total_byte_size += byte_sizes[m_idx++];
    };

    storage.resize(total_byte_size);
    std::cout << "storage of " << total_byte_size << " bytes in total\n\n";

    // Loop over storage vectors
    size_t offset = 0;
    m_idx = 0;
    [:expand(nonstatic_data_members_of(^T)):] >> [&]<auto member> {
      // reading sizes directly in queue injection doesn't seem to work?
      // results in a "cannot capture sizes" error
      auto sov_size = sizes[m_idx];

      consteval {
        auto name = name_of(member);
        auto type = get_scalar_type(type_of(member));

        // Assign required bytes to storage vector e.g.,
        //    _x = std::span(reinterpret_cast<double*>(storage.data()) + offset,
        //                   sizes[m_idx]);
        queue_injection(^{
          \id(member_prefix, name) = std::span(reinterpret_cast<[: \(type):] *>(storage.data() + offset), sov_size);
        });
      }
      offset += byte_sizes[m_idx++];

      // Fill storage spans
      if constexpr (type_is_eigen(type_of(member))) {
        fill_sov_matrix<member>(data);
      } else if constexpr (type_is_container(type_of(member))) {
        fill_sov_container<member>(data);
      } else {
        fill_sov_scalar<member>(data);
      }
    };
  }

  size_t size() const { return m_size; }

  aos_view operator[](const size_t idx) const {
    consteval { // gather references to sov elements
      std::meta::list_builder member_data_tokens{};
      for (auto member : nonstatic_data_members_of(^T)) {
        auto name = name_of(member);
        auto type = type_of(member);
        auto sov_name = ^{ \id(member_prefix, name) };

        if (type_is_eigen(type)) {
          auto dim = extract<size_t>(template_arguments_of(type)[1]);
          auto extents = ^{ typename aos_view::\id(name, extents_suffix) };
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

      // Injects:
      //     return aos_view(.x = _x[idx], .v = _v.subspan(_v_md[idx].offset, _v_md[idx].size));
      queue_injection(^{ return aos_view{\tokens(member_data_tokens)}; });
    }
  }

  template <typename View>
  View get_view() const {
    throw std::invalid_argument{"Unsupported view"};
  }

  template <typename View>
  View get_view(const size_t idx) const
    requires std::is_same_v<View, aos_view>
  {
    return operator[](idx);
  }

  template <typename View>
  View get_view() const
    requires std::is_same_v<View, soa_view>
  {
    // consteval {
    //   std::meta::list_builder member_data_tokens{};
    //   for (auto member : nonstatic_data_members_of(^T)) {
    //     auto name = name_of(member);
    //     auto sov_name = ^{ \id(member_prefix, name) };
    //     member_data_tokens += ^{ .\id(name) = \tokens(sov_name) };
    //   }

    //   queue_injection(^{ return soa_view{\tokens(member_data_tokens)}; });
    // }
  }
};
} // namespace rmpp
#endif