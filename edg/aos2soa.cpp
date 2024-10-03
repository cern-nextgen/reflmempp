// Simple SoA structure with an AoS interface using P2996 reflection and P3294 token injection

#include <experimental/meta>
#include <iostream>

namespace rmpp {
using namespace std::literals;

consteval void gen_sov_members(auto t) {
    for (auto member : nonstatic_data_members_of(t)) {
        auto vec_member = ^{
          \id("_"sv, name_of(member))
        };

        queue_injection(^{
          std::vector<typename[:\(type_of(member)):]> \tokens(vec_member);
        });
    }
}

consteval void gen_sor_members(auto t) {
    for (auto member : nonstatic_data_members_of(t)) {
        queue_injection(^{
          const typename[:\(type_of(member)):] & \id(name_of(member));
        });
    }
}

template <typename T>
class vector {
    // ------------ generate -----------
    //   private:
    //      std:vector<double> _x, _y, _z, _value;
    //      struct aos_view {
    //          double &x, &y, &z, &value;
    //      }
    //
    //   public:
    //     size_t size() {
    //          return _x.size();
    //     }
    //
    //     void push_back(T elem) {
    //          _x.push_back(elem.x);
    //          _y.push_back(elem.y);
    //          _z.push_back(elem.z);
    //          _value.push_back(elem.value);
    //     }
    //
    //     auto operator[](std::size_t idx) {
    //          return aos_view(_x[idx], _y[idx], _z[idx], _value[idx]);
    //     }
   private:
    consteval { gen_sov_members(^T); }

    struct aos_view {
        consteval { gen_sor_members(^T); }
    };

   public:
    size_t size() const {
        consteval {
            auto first = nonstatic_data_members_of(^T)[0];
            queue_injection(^{
              return \id("_"sv, name_of(first)).size();
            });
        }
    }

    void push_back(T const& elem) & {
        consteval {
            // push back the data into sov
            for (auto member : nonstatic_data_members_of(^T)) {
                auto name = name_of(member);

                queue_injection(^{
                  \id("_"sv, name).push_back(elem.\id(name));
                });
            }
        }
    }

    aos_view operator[](std::size_t idx) const {
        consteval {
            // gather references to sov elements
            std::meta::list_builder member_data_tokens{};
            for (auto member : nonstatic_data_members_of(^T)) {
                auto name = name_of(member);

                member_data_tokens += ^{
                  .\id(name) = \id("_"sv, name)[idx]
                };
            }
            // __report_tokens(member_data_tokens);

            queue_injection(^{
              return aos_view{\tokens(member_data_tokens)};
            });
        }
    }
};
}  // namespace rmpp

// dummy
struct data {
    double x, y, z, value;
};

int main() {
    rmpp::vector<data> maos;

    data e1 = {0, 1, 2, 3};
    data e2 = {4, 5, 6, 7};
    data e3 = {8, 9, 10, 11};

    maos.push_back(e1);
    maos.push_back(e2);
    maos.push_back(e3);

    std::cout << "maos.size = " << maos.size() << "\n";
    for (size_t i = 0; i != maos.size(); ++i) {
        std::cout << "maos[" << i << "] = ( x:" << maos[i].x << ", y:" << maos[i].y
                  << ", z:" << maos[i].z << ", value:" << maos[i].value << ")\n";
    }
}
