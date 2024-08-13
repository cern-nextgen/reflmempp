// SoA structure with an AoS interface using P2996 reflection and P3294 token injection.
// Each array in the SoA is allocated in a contiguous storage container.
// Run via: https://godbolt.org/z/qjMTWTaaY

#include <concepts>
#include <experimental/meta>
#include <iostream>
#include <span>
#include <type_traits>

template <class T>
static constexpr bool is_vector_v = requires {
    requires std::same_as<std::decay_t<T>, std::vector<typename std::decay_t<T>::value_type>>;
};

consteval auto type_is_vector(std::meta::info r) -> bool {
    return extract<bool>(std::meta::substitute(^is_vector_v, {
                                                                 r}));
}

namespace mds {
using namespace std::literals;

consteval auto gen_sov_members(std::meta::info t) -> void {
    for (auto member : nonstatic_data_members_of(t)) {
        auto vec_member = ^{
          \id("_"sv, name_of(member))
        };

        queue_injection(^{
          std::span<typename[:\(type_of(member)):]> \tokens(vec_member);
        });
    }
}

consteval auto gen_sor_members(std::meta::info t) -> void {
    for (auto member : nonstatic_data_members_of(t)) {
        queue_injection(^{
          const typename[:\(type_of(member)):] & \id(name_of(member));
        });
    }
}

template <typename T, size_t Alignment>
class vector {
    // ------------ generate -----------
    //   private:
    //      std::vector<std::byte> storage;
    //      std::span<double> _x, _y, _z, _value;
    //      struct aos_view {
    //          double &x, &y, &z, &value;
    //      }
   private:
    std::vector<std::byte> storage;

    consteval { gen_sov_members(^T); }
    std::vector<size_t> sizes;  // Number of data elements per data vector

    struct aos_view {
        consteval { gen_sor_members(^T); }
    };

    // Helper function to compute aligned size
    constexpr inline size_t align_size(size_t size, size_t alignment) {
        return ((size + alignment - 1) / alignment) * alignment;
    }

    // Compute the number of bytes needed for each storage vector and the
    // total number of storage bytes.
    template <std::meta::info Member>
    constexpr auto compute_sizes(const std::initializer_list<T> data, size_t& size,
                                 size_t& byte_size) -> void {
        // other types with size()?
        if constexpr (is_vector_v<typename[:type_of(Member):]>) {
            for (auto elem : data) {
                byte_size +=
                    align_size(sizeof(typename[:type_of(Member):][data.size()]), Alignment);
                size += elem.[:Member:].size();
            }
        } else {
            byte_size = align_size(sizeof(typename[:type_of(Member):][data.size()]), Alignment);
            size = data.size();
        }

        std::cout << "_" << name_of(Member) << " = " << size << " elements in " << byte_size
                  << " bytes\n";
    }

   public:
    vector(std::initializer_list<T> data) {
        auto n_members = [:std::meta::reflect_value(nonstatic_data_members_of(^T).size()):];

        std::vector<size_t> byte_sizes(n_members);  // this info will be kept in std::span
        sizes.resize(n_members);
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
        [:expand(nonstatic_data_members_of(^T)):] >> [&, this]<auto e> {
            consteval {
                auto name = name_of(e);
                auto type = type_of(e);

                // Assign required bytes to storage vector
                // e.g., _x = std::span(reinterpret_cast<double*>(storage.data()),
                //                      byte_sizes[m_idx] / sizeof(double));
                queue_injection(^{
                  \id("_"sv, name) =
                      std::span(reinterpret_cast<typename[: \(type):]*>(storage.data() + offset),
                                byte_sizes[m_idx] / sizeof(typename[: \(type):]));
                });
            }
            offset += byte_sizes[m_idx];

            // Fill storage vector
            size_t e_idx = 0;
            for (auto elem : data) {
                consteval {
                    // e.g, new (&_x[e_idx]) double(elem.x);
                    queue_injection(^{
                      new (&\id("_"sv, name_of(e))[e_idx]) decltype(elem.\id(name_of(e)))(
                          elem.\id(name_of(e)));
                    });
                }
                e_idx++;
            }
        };
    }

    auto size() const -> std::size_t { return sizes[0]; }

    auto operator[](std::size_t m_idx) const -> aos_view {
        consteval {
            // gather references to sov elements
            std::meta::list_builder member_data_tokens{};
            for (auto member : nonstatic_data_members_of(^T)) {
                auto name = name_of(member);

                member_data_tokens += ^{
                  .\id(name) = \id("_"sv, name)[m_idx]
                };
            }

            // Injects: return aos_view(_x[m_idx], _y[m_idx], _z[m_idx], _value[m_idx]);
            queue_injection(^{
              return aos_view{\tokens(member_data_tokens)};
            });
        }
    }
};
}  // namespace mds

// dummy
struct data {
    double x, y, z, value;
    std::vector<int> a, b;
};

template <typename T>
void print_vector(T& v) {
    std::cout << "{";
    for (size_t i = 0; i < v.size(); i++) {
        if (i != 0) std::cout << ", ";
        std::cout << v[i];
    }
    std::cout << "}, ";
}

template <typename T>
auto test(T& t) -> size_t {
    return sizeof(t);
}

int main() {
    data e1 = {0, 1, 2, 3, {100, 101, 102, 103}, {}};
    data e2 = {4, 5, 6, 7, {201, 202}, {2001, 2002}};
    data e3 = {8, 9, 10, 11, {301}, {3001, 3002, 3003}};

    mds::vector<data, 64> maos = {e1, e2, e3};

    std::cout << "maos.size = " << maos.size() << "\n";
    for (size_t i = 0; i != maos.size(); ++i) {
        std::cout << "maos[" << i << "] = (";

        consteval {
            for (auto member : nonstatic_data_members_of(^data)) {
                queue_injection(^{
                  std::cout << \(name_of(member)) << ": ";
                });

                if (type_is_vector(type_of(member))) {
                    queue_injection(^{
                      print_vector<decltype(maos[i].\id(name_of(member)))>(
                          maos[i].\id(name_of(member)));
                    });
                } else {
                    queue_injection(^{
                      std::cout << maos[i].\id(name_of(member)) << ", ";
                    });
                }
            }
        }

        std::cout << ")\n";
    }
    std::cout << "\n";

    for (size_t i = 0; i != maos.size(); ++i) {
        std::cout << "maos[" << i << "] = (";

        consteval {
            for (auto member : nonstatic_data_members_of(^data)) {
                queue_injection(^{
                  std::cout << \(name_of(member)) << ": " << &maos[i].\id(name_of(member)) << ",";
                });
            }
        }

        std::cout << ")\n";
    }

    return 0;
}