// SoA structure with an AoS interface using P2996 reflection and P3294 token
// injection. Each array in the SoA is allocated in a contiguous storage
// container.
// Run here: https://godbolt.org/z/P613hh8MG

#define __cpp_lib_reflection 20240815

#include "mdspan.h"
#include "utils.h"

#include <array>
#include <concepts>
#include <experimental/meta>
#include <iostream>
#include <span>
#include <type_traits>

using namespace std::literals::string_view_literals;

///
// Magic Data Structure
///
namespace mds {

consteval auto gen_sov_members(std::meta::info t) -> void {
  for (auto member : nonstatic_data_members_of(t)) {
    auto vec_member = ^{
      \id("_"sv, name_of(member))
    };

    auto type = type_of(member);
    if (type_is_container(type)) {
      type = get_scalar_type(type);
      queue_injection(^{
        std::vector<sov_metadata> \id("_"sv, name_of(member), "_md"sv);
      });
    }

    queue_injection(^{
      std::span<typename[:\(type):]> \tokens(vec_member);
    });
  }
}

consteval auto gen_sor_members(std::meta::info t) -> void {
  for (auto member : nonstatic_data_members_of(t)) {
    auto type = type_of(member);
    auto scalar_type = get_scalar_type(type);
    if (type_is_eigen(type)) {
      auto dim = reflect_value(template_arguments_of(type)[1]);
      queue_injection(^{
        const std::mdspan<typename[:\(scalar_type):], std::extents<size_t, [: \(dim):], [: \(dim):]>>  \id(
            name_of(member));
      });
    } else if (type_is_container(type)) {
      queue_injection(^{
        const std::span<typename[:\(scalar_type):]>  \id(name_of(member));
      });
    } else { // scalar
      queue_injection(^{
        const typename[:\(type):] & \id(name_of(member));
      });
    }
  }
}

template <typename T, size_t Alignment> class vector {
private:
  std::vector<std::byte> storage;
  size_t _size; // Number of elements

public: // internal data public for debugging
  struct sov_metadata {
    size_t offset, size;
    friend std::ostream &operator<<(std::ostream &os, const sov_metadata &obj) {
      return os << "{" << obj.offset << ", " << obj.size << "}";
    }
  };
  consteval { gen_sov_members(^T); }

  std::vector<size_t> byte_sizes; // Size of each SoV including alignment padding

  struct aos_view {
    consteval { gen_sor_members(^T); }
  };

  // Helper function to compute aligned size
  constexpr inline size_t align_size(size_t size, size_t alignment) {
    return ((size + alignment - 1) / alignment) * alignment;
  }

  // Compute the number of bytes needed for each storage vector and the total number of storage bytes.
  template <std::meta::info Member>
  auto compute_sizes(const std::initializer_list<T> data, size_t &size, size_t &byte_size) -> void {
    if constexpr (type_is_container(type_of(Member))) {
      using vec_type = typename[:type_of(Member):] ::value_type;
      for (auto &elem : data) {
        auto n_elements = elem.[:Member:].size();
        consteval {
          queue_injection(^{
            \id("_"sv, name_of(Member), "_md"sv).push_back({.offset = size, .size = n_elements});
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

public:
  vector(std::initializer_list<T> data) {
    auto n_members = [:std::meta::reflect_value(nonstatic_data_members_of(^T).size()):];
    _size = data.size();

    std::vector<size_t> sizes(n_members);
    byte_sizes.resize(n_members);
    size_t total_byte_size = 0;
    size_t m_idx = 0;
    [:expand(nonstatic_data_members_of(^T)):] >> [&]<auto e> {
      compute_sizes<e>(data, sizes[m_idx], byte_sizes[m_idx]);
      total_byte_size += byte_sizes[m_idx++];
    };

    storage.resize(total_byte_size);
    std::cout << "storage of " << total_byte_size << " bytes in total\n\n";

    // Loop over storage vectors
    size_t offset = 0;
    m_idx = 0;
    [:expand(nonstatic_data_members_of(^T)):] >> [&]<auto e> {
      // reading sizes directly in queue injection doesn't seem to work?
      // results in a "cannot capture sizes" error
      auto sov_size = sizes[m_idx];

      consteval {
        auto name = name_of(e);
        auto type = get_scalar_type(type_of(e));

        // Assign required bytes to storage vector e.g.,
        //    _x = std::span(reinterpret_cast<double*>(storage.data()) + offset,
        //                   sizes[m_idx]);
        queue_injection(^{
          \id("_"sv, name) = std::span(reinterpret_cast<[: \(type):] *>(storage.data() + offset), sov_size);
        });
      }
      offset += byte_sizes[m_idx++];

      // Fill storage spans
      size_t e_idx = 0;
      for (auto &elem : data) {
        if constexpr (type_is_container(type_of(e))) {
          using vec_type = typename[:type_of(e):] ::value_type;

          for (size_t i = 0; i < elem.[:e:].size(); i++) {
            consteval {
              // e.g, new (&_a[e_idx]) double(elem.x);
              queue_injection(^{
                new (&\id("_"sv, name_of(e))[e_idx]) vec_type(elem.[:e:][i]);
              });
            }
            e_idx++;
          }
        } else {
          consteval {
            // e.g, new (&_x[e_idx]) double(elem.x);
            queue_injection(^{
              new (&\id("_"sv, name_of(e))[e_idx]) decltype(elem.[:e:])(elem.[:e:]);
            });
          }
          e_idx++;
        }
      }
    };
  }

  auto size() const -> std::size_t { return _size; }

  auto operator[](std::size_t m_idx) const -> aos_view {
    consteval {
      // gather references to sov elements
      std::meta::list_builder member_data_tokens{};
      for (auto member : nonstatic_data_members_of(^T)) {
        auto name = name_of(member);
        auto sov_name = ^{
          \id("_"sv, name)
        };

        if (type_is_container(type_of(member))) {
          auto md_name = ^{
            \id("_"sv, name, "_md"sv)
          };
          member_data_tokens += ^{
            .\id(name) = \tokens(sov_name).subspan(\tokens(md_name)[m_idx].offset, \tokens(md_name)[m_idx].size)
          };
        } else {
          member_data_tokens += ^{
            .\id(name) = \tokens(sov_name)[m_idx]
          };
        }
      }

      // Injects:
      //     return aos_view(.x = _x[idx], .v = _v.subspan(_v_md[idx].offset, _v_md[idx].size));
      queue_injection(^{
        return aos_view{\tokens(member_data_tokens)};
      });
    }
  }
};
} // namespace mds

// dummy
struct data {
  double x;
  std::vector<int> v;
  // std::vector<std::vector<double>> m;
  EigenMatrix<float, 2> m;
};

int main() {
  data e1 = {0, {10, 11, 12, 13}, {{{100, 101}, {102, 103}}}};
  data e2 = {4, {20}, {{{200, 201}, {202, 203}}}};
  data e3 = {8, {30, 31}, {{{300, 301}, {302, 303}}}};

  mds::vector<data, 64> maos = {e1, e2, e3};

  std::cout << "maos.size = " << maos.size() << "\n";
  for (size_t i = 0; i != maos.size(); ++i) {
    std::cout << "maos[" << i << "] = (";

    [:expand(nonstatic_data_members_of(^decltype(maos[i]))):] >> [&]<auto e> {
      std::cout << name_of(e) << ": ";
      print_member(maos[i].[:e:]);
    };

    std::cout << ")\taddr = ";

    [:expand(nonstatic_data_members_of(^decltype(maos[i]))):] >> [&]<auto e> {
      std::cout << name_of(e) << ": ";
      print_member_addr(maos[i].[:e:]);
    };
  }

  std::cout << "\n";

  //// print underlying data ////

  // Edison Design Group C/C++ Front End, version 6.6 (Jul 29 2024 17:25:25)
  // - accessible_members_of is undefined
  // - nonstatic_data_members_of doesn't accept filters
  [:expand(members_of(^mds::vector<data, 64>, std::meta::is_nonstatic_data_member, std::meta::is_accessible)
           ):] >> [&]<auto e> {
    std::cout << name_of(e) << " = ";
    print_member(maos.[:e:]);
    std::cout << "\taddr = ";
    print_member_addr(maos.[:e:]);
  };

  return 0;
}
