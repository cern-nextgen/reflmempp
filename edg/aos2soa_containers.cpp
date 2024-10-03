// SoA structure with an AoS interface using P2996 reflection and P3294 token
// injection. Each array in the SoA is allocated in a contiguous storage
// container.

#define __cpp_lib_reflection 20240815

#include "rmpp.h"

#include <array>
#include <concepts>
#include <experimental/meta>
#include <iostream>
#include <span>
#include <type_traits>

using namespace std::literals::string_view_literals;

// dummy
struct data {
  double x;
  std::vector<int> v;
  EigenMatrix<float, 2> m;
};

int main() {
  data e1 = {0, {10, 11, 12, 13}, {{{100, 101}, {102, 103}}}};
  data e2 = {4, {20}, {{{200, 201}, {202, 203}}}};
  data e3 = {8, {30, 31}, {{{300, 301}, {302, 303}}}};

  rmpp::vector<data, 64> maos = {e1, e2, e3};

  std::cout << "maos.size = " << maos.size() << "\n";
  for (size_t i = 0; i != maos.size(); ++i) {
    std::cout << "maos[" << i << "] = ( x:" << maos[i].x << ", v: {";
    print_member(maos[i].v);
    std::cout << "}, m: {";
    auto md = maos[i].m;
    for (size_t j = 0; j < md.extent(0); j++) {
      if (j != 0) std::cout << ", ";
      std::cout << "{";
      for (size_t k = 0; k < md.extent(1); k++) {
        if (k != 0) std::cout << ", ";
        std::cout << md(j, k);
      }
      std::cout << "}";
    }
    std::cout << "} )\n";
  }

  std::cout << "\n";

  //// print underlying data ////

  // Edison Design Group C/C++ Front End, version 6.6 (Jul 29 2024 17:25:25)
  // - accessible_members_of is undefined
  // - nonstatic_data_members_of doesn't accept filters
  [:expand(members_of(^rmpp::vector<data, 64>, std::meta::is_nonstatic_data_member, std::meta::is_accessible)
           ):] >> [&]<auto e> {
    std::cout << "\n" << name_of(e) << " = ";
    print_member(maos.[:e:]);
    std::cout << "\n\taddr = ";
    print_member_addr(maos.[:e:]);
  };

  std::cout << "\n";

  return 0;
}
