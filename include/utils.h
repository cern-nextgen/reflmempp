#ifndef UTILS_H
#define UTILS_H

#include <concepts>
#include <iostream>
#include <span>
#include <type_traits>
#include <vector>

// #include "mdspan.h" // doesnt't work with eccp? fails template deduction with md[j][k], think it's related to c++ std used
#include "kokkos/mdspan.h"

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

template <typename T>
concept Struct = std::is_class_v<T> && !std::is_union_v<T>;

template <class T>
concept Span = std::same_as<std::decay_t<T>, std::span<typename std::decay_t<T>::value_type>>;

// https://stackoverflow.com/a/60491447
template <class ContainerType>
concept Container = Span<ContainerType> || requires(ContainerType a, const ContainerType b) {
  requires std::regular<ContainerType>;
  requires std::swappable<ContainerType>;
  requires std::destructible<typename ContainerType::value_type>;
  requires std::same_as<typename ContainerType::reference, typename ContainerType::value_type &>;
  requires std::same_as<typename ContainerType::const_reference, const typename ContainerType::value_type &>;
  // requires std::forward_iterator<typename ContainerType::iterator>;
  // requires std::forward_iterator<typename
  // ContainerType::const_iterator>;
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

template <typename T>
concept NestedContainer = Container<T> && Container<typename T::value_type>;

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

// Multiple size of dimensions for nested arrays.
template <typename T, size_t N, size_t M>
struct get_array_size<std::array<std::array<T, M>, N>> {
  constexpr static size_t size = N * get_array_size<std::array<T,M>>::size;
};

template <typename T>
struct get_inner_type {
  using type = T;
};

// Recursively get the scalar type of a container type.
template <Container T>
  requires requires { typename T::value_type; }
struct get_inner_type<T> {
  using type = typename get_inner_type<typename T::value_type>::type;
};

template <typename T>
using inner_type = get_inner_type<T>::type;

template <class T>
concept Eigen = std::same_as<T, EigenMatrix<typename get_inner_type<T>::type, get_array_size<T>::size>>;
// requires std::same_as<T,
// EigenMatrix<typename[:get_scalar_type(type_decay(^^T)):],
// get_array_size<T>::size>>;

///
// Methods taking std::meta::info
///

#ifdef __cpp_lib_reflection
namespace __impl {
  template<auto... vals>
  struct replicator_type {
    template<typename F>
      constexpr auto operator>>(F body) const -> decltype(auto) {
        return body.template operator()<vals...>();
      }
  };

  template<auto... vals>
  replicator_type<vals...> replicator = {};
}

template<typename R>
consteval auto expand_all(R range) {
  std::vector<std::meta::info> args;
  for (auto r : range) {
    args.push_back(reflect_value(r));
  }
  return substitute(^^__impl::replicator, args);
}

consteval auto nsdms(std::meta::info type) -> std::vector<std::meta::info> {
#ifdef __clang__
    return nonstatic_data_members_of(type, std::meta::access_context::current());
#else
    return nonstatic_data_members_of(type);
#endif
}

template <class T>
concept Reflection = std::same_as<std::meta::info, T>;

consteval bool is_reflection(std::meta::info r) {
  return extract<bool>(std::meta::substitute(^^Reflection, {r}));
}

consteval bool type_is_struct(std::meta::info r) {
  return extract<bool>(std::meta::substitute(^^Struct, {r}));
}

consteval bool type_is_container(std::meta::info r) {
  return extract<bool>(std::meta::substitute(^^Container, {r}));
}

consteval bool type_is_nested_container(std::meta::info r) {
  return extract<bool>(std::meta::substitute(^^NestedContainer, {r}));
}

consteval bool type_is_eigen(std::meta::info r) {
  return extract<bool>(std::meta::substitute(^^Eigen, {r}));
}

consteval std::meta::info get_scalar_type(std::meta::info t) {
  // if (type_is_container(t)) { return
  // get_scalar_type(template_arguments_of(t)[0]); } return t;

  // Can i do this with just without inner_type and just get_inner_type
  // instead?
  return substitute(^^inner_type, {t});
}

///
// Print utilities
///

consteval void __report_and_inject(std::meta::info tokens) {
  __report_tokens(tokens);
  queue_injection(^^{ \tokens(tokens)});
}

auto indent_string(int indent) {
  std::string r;
  for (int i = 0; i < indent; i++) {
    r += std::string(" ");
  }
  return r;
}

template <typename T>
void print_member(const T &v) {
  if constexpr (Container<T>) {
    std::cout << "{";
    for (size_t i = 0; i < v.size(); i++) {
      if (i != 0) std::cout << ", ";
      print_member(v[i]);
    }
    std::cout << "}";
  } else if constexpr (Struct<T>) {
    std::cout << identifier_of(^^T) << "{ ";

    int i = 0;
    [:expand(nsdms(^^T)):] >> [&]<auto e> {
      if (i != 0) std::cout << ", ";
      print_member(v.[:e:]);
      i++;
    };

    std::cout << " }";
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
  } else if constexpr (Struct<T>) {
    std::cout << identifier_of(^^T) << "{ ";

    int i = 0;
    [:expand(nsdms(^^T)):] >> [&]<auto e> {
      if (i != 0) std::cout << ", ";
      print_member_addr(v.[:e:]);
      i++;
    };

    std::cout << " }";
  } else {
    std::cout << (long long)&v;
  }
}

void print_soa_as_aos(auto &aos) {
  std::cout << "soa.size = " << aos.size();
  for (size_t i = 0; i != aos.size(); ++i) {
    std::cout << "\naos[" << i << "] = (\n";

    [:expand(nsdms(^^decltype(aos[i]))):] >> [&]<auto e> {
      std::cout << "\t" << identifier_of(e) << ": ";
      print_member(aos[i].[:e:]);
      std::cout << "\n";
    };

    [:expand(nsdms(^^decltype(aos[i]))):] >> [&]<auto e> {
      std::cout << "\t" << identifier_of(e) << " ADDR: ";
      print_member_addr(aos[i].[:e:]);
      std::cout << "\n";
    };
  }
}
#endif

#endif