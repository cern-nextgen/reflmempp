#ifndef UTILS_H
#define UTILS_H

#include <concepts>
#include <iostream>
#include <type_traits>
#include <vector>

// #include "mdspan.h" // doesnt't work with eccp? fails template deduction with md[j][k], think it's related to c++ std used
#include "mdspan/mdspan.hpp"

#ifdef __cpp_lib_reflection
#include <experimental/meta>
#endif

///
// Containers
///

template <class... T>
using mdspan = Kokkos::mdspan<T...>;
template <typename _IndexType, size_t... _Extents>
using extents = Kokkos::extents<_IndexType, _Extents...>;

using layout_stride = Kokkos::layout_stride;

template <class T>
concept Span = std::same_as<std::decay_t<T>, std::span<typename std::decay_t<T>::value_type>>;

template <class T>
concept MDSpan = std::same_as<std::decay_t<T>,
                              mdspan<typename std::decay_t<T>::value_type, typename std::decay_t<T>::extents_type,
                                     typename std::decay_t<T>::layout_type, typename std::decay_t<T>::accessor_type>>;

// https://stackoverflow.com/a/60491447
template <class ContainerType>
concept Container = Span<ContainerType> || requires(ContainerType a, const ContainerType b) {
  requires std::regular<ContainerType>;
  requires std::swappable<ContainerType>;
  requires std::destructible<typename ContainerType::value_type>;
  requires std::same_as<typename ContainerType::reference, typename ContainerType::value_type &>;
  requires std::same_as<typename ContainerType::const_reference, const typename ContainerType::value_type &>;
  // requires std::forward_iterator<typename ContainerType::iterator>;
  // requires std::forward_iterator<typename ContainerType::const_iterator>;
  requires std::signed_integral<typename ContainerType::difference_type>;
  requires std::same_as<typename ContainerType::difference_type,
                        typename std::iterator_traits<typename ContainerType::iterator>::difference_type>;
  requires std::same_as<typename ContainerType::difference_type,
                        typename std::iterator_traits<typename ContainerType::const_iterator>::difference_type>;
  { a.begin() } -> std::same_as<typename ContainerType::iterator>;
  { a.end() } -> std::same_as<typename ContainerType::iterator>;
  { b.begin() } -> std::same_as<typename ContainerType::const_iterator>;
  { b.end() } -> std::same_as<typename ContainerType::const_iterator>;
  { a.cbegin() } -> std::same_as<typename ContainerType::const_iterator>;
  { a.cend() } -> std::same_as<typename ContainerType::const_iterator>;
  { a.size() } -> std::same_as<typename ContainerType::size_type>;
  { a.max_size() } -> std::same_as<typename ContainerType::size_type>;
  { a.empty() } -> std::same_as<bool>;
};

///
// Eigen Matrices
///

// something more generic?
template <typename T, size_t D>
using EigenMatrix = std::array<std::array<T, D>, D>;

template <typename>
struct get_array_size;
template <typename T, size_t S>
struct get_array_size<std::array<T, S>> {
  constexpr static size_t size = S;
};

template <typename T>
struct get_inner_type {
  using type = T;
};

template <Container T>
struct get_inner_type<T> {
  using type = get_inner_type<typename T::value_type>::type;
};

template <class T>
concept Eigen = std::same_as<T, EigenMatrix<typename get_inner_type<T>::type, get_array_size<T>::size>>;
// requires std::same_as<T, EigenMatrix<typename[:get_scalar_type(type_decay(^T)):], get_array_size<T>::size>>;

///
// Methods taking std::meta::info
///

#ifdef __cpp_lib_reflection
consteval auto type_is_container(std::meta::info r) -> bool {
  return extract<bool>(std::meta::substitute(^Container, {r}));
}

consteval auto type_is_eigen(std::meta::info r) -> bool {
  return extract<bool>(std::meta::substitute(^Eigen, {r}));
}

template <typename T>
using inner_type = get_inner_type<T>::type;

consteval auto get_scalar_type(std::meta::info t) -> std::meta::info {
  // if (type_is_container(t)) { return get_scalar_type(template_arguments_of(t)[0]); }
  // return t;

  // Can i do this with just without inner_type and just get_inner_type instead?
  return substitute(^inner_type, {t});
}
#endif

///
// Print utilities
///

template <typename T>
void print_member(const T &v) {
  if constexpr (Container<T>) {
    std::cout << "{";
    for (size_t i = 0; i < v.size(); i++) {
      if (i != 0) std::cout << ", ";
      print_member(v[i]);
    }
    std::cout << "}";
  } else if constexpr (MDSpan<T>) { // FIXME:
    //   std::cout << "{";
    //   for (size_t i = 0; i < v.extent(0); i++) {
    //     if (i != 0) std::cout << ", ";
    //     std::cout << "{";
    //     for (size_t j = 0; j < v.extent(1); j++) {
    //       if (j != 0) std::cout << ", ";
    //       std::cout << v[i][j];
    //     }
    //     std::cout << "}";
    //   }
    //   std::cout << "}";
  } else {
    std::cout << v;
  }
}

template <typename T>
void print_member_addr(const T &v) {
  if constexpr (Container<T>) {
    std::cout << "{";
    for (size_t i = 0; i < v.size(); i++) {
      if (i != 0) std::cout << ", ";
      print_member_addr(v[i]);
    }
    std::cout << "}";
  } else if constexpr (MDSpan<T>) {

  } else {
    std::cout << (long long)&v;
  }
}

#endif