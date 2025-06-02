#include "mdspan.h"
#include "mdspan/mdspan.hpp"
#include <iostream>
#include <vector>

// #include <print>

int main(int argc, char const *argv[]) {
  std::vector<float> data{1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4};
  using StdExtents = std::extents<size_t, 2, 2>;

  std::mdspan<float, StdExtents, std::layout_stride> stdtest{data.data(), {StdExtents{}, std::array<size_t, 2>{3 * 2, 3}}};
  for (size_t j = 0; j < stdtest.extent(0); j++) {
    if (j != 0) std::cout << ", ";
    std::cout << "{";
    for (size_t k = 0; k < stdtest.extent(1); k++) {
      if (k != 0) std::cout << ", ";
      std::cout << stdtest[j, k];
      // std::cout << stdtest(j, k);
    }
    std::cout << "}";
  }

  using KkExtents = Kokkos::extents<size_t, 2, 2>;
  Kokkos::mdspan<float, KkExtents, Kokkos::layout_stride> kktest{data.data(),
                                                                 {KkExtents{}, std::array<size_t, 2>{3 * 2, 3}}};

  // expected: {{1,2}, {3,4}}
  std::cout << "{";
  for (size_t j = 0; j < kktest.extent(0); j++) {
    if (j != 0) std::cout << ", ";
    std::cout << "{";
    for (size_t k = 0; k < kktest.extent(1); k++) {
      if (k != 0) std::cout << ", ";
      std::cout << kktest[j, k];
      // std::cout << kktest(j, k);
    }
    std::cout << "}";
  }
  std::cout << "}\n";

// std::__report_constexpr_value();
  // std::cout << __GNUC__ << "<<<<<<<<<<<<<<<\n";
  // std::println("test");

  return 0;
}


