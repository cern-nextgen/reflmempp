#ifndef WRAPPER_H
#define WRAPPER_H

/**
 * This example is adapted from Oliver Rietmann's SoA wrapper approach:
 *     https://github.com/cern-nextgen/wp1.7-soa-wrapper/commit/21d5b849ef7336bd56ebd896c9f34339a5aa75e3
 * This version reduces code duplication by using C++26 reflection features.
 */

#include <experimental/meta>
#include <ranges>

template <class T>
using value = T;
template <class T>
using reference = T &;
template <class T>
using const_reference = const T &;
template <class T>
using pointer = T *;
template <class T>
using const_pointer = const T *;

//////////////// Reflection utilities

template <class S>
constexpr std::size_t count_members() {
  return nonstatic_data_members_of(^^S, std::meta::access_context::unchecked()).size();
}

consteval auto nsdms(std::meta::info type) -> std::vector<std::meta::info> {
  return nonstatic_data_members_of(type, std::meta::access_context::unchecked());
}

template <class F>
consteval auto transform_members(std::meta::info type, F f) {
  return nsdms(type) | std::views::transform([=](std::meta::info member) {
           return data_member_spec(f(type_of(member)), {.name = identifier_of(member)});
         });
}

namespace __impl {
template <auto... vals>
struct replicator_type {
  template <typename F>
  constexpr auto operator>>(F body) const -> decltype(auto) {
    return body.template operator()<vals...>();
  }
};

template <auto... vals>
replicator_type<vals...> replicator = {};
} // namespace __impl

template <typename R>
consteval auto expand_all(R range) {
  std::vector<std::meta::info> args;
  for (auto r : range) {
    args.push_back(reflect_constant(r));
  }
  return substitute(^^__impl::replicator, args);
}

//////////////// Wrapper generator

template <class SF>
struct RandomAccessAt {
  size_t i;
  template <class... Args>
  constexpr SF operator()(Args &...args) const {
    return {args[i]...};
  }
  template <class... Args>
  constexpr SF operator()(const Args &...args) const {
    return {args[i]...};
  }
};

template <class SF>
struct AggregateConstructor {
  template <class... Args>
  constexpr SF operator()(Args &...args) const {
    return {args...};
  }
  template <class... Args>
  constexpr SF operator()(const Args &...args) const {
    return {args...};
  }
};

template <typename S, template <class> class F>
struct WrapperGeneratorBase {
  struct Base;

  consteval {
    define_aggregate(
        ^^Base, transform_members(^^S, [](std::meta::info type) { return substitute(^^F, {remove_cvref(type)}); }));
  }

  class Wrapper : public Base {
  public:
    template <class FunctionObject>
    auto apply(FunctionObject &&f) {
      auto construct_output = [&]<size_t... Is>(std::index_sequence<Is...>) {
        return f(this->[:nsdms(^^Base)[Is]:]...);
      };
      constexpr auto indices = std::make_index_sequence<count_members<Base>()>{};
      return construct_output(indices);
    }

    //////// Random Access operators

    S operator[](size_t i) { return apply(RandomAccessAt<S>{i}); }

    WrapperGeneratorBase<S, F>::Wrapper operator[](size_t i) const {
      return apply(RandomAccessAt<WrapperGeneratorBase<S, F>::Wrapper>{i});
    }

    ////// Constructors

    Wrapper() = default;

    template <typename... T>
    Wrapper(std::initializer_list<T>... args) : Base(args...) {}

    template <typename... T>
    Wrapper(T &&...t) : Base(std::forward<T>(t)...) {}

    Wrapper(const Base &b) : Base(b) {}

    ////// Conversion constructors
    template <typename T>
      requires(parent_of(^^T) != parent_of(^^Wrapper))
    Wrapper(T &other) : Wrapper(other.apply(AggregateConstructor<Base>{})) {}

    template <typename T>
      requires(parent_of(^^T) != parent_of(^^Wrapper))
    Wrapper(const T &other) : Wrapper(other.apply(AggregateConstructor<Base>{})) {}
  };
};

template <typename T, template <class> class F>
struct WrapperGenerator : public WrapperGeneratorBase<T, F> {
  using Base = WrapperGeneratorBase<T, F>;
  using typename Base::Wrapper;
};

template <typename T>
struct WrapperGenerator<T, pointer> : public WrapperGeneratorBase<T, pointer> {
  using Base = WrapperGeneratorBase<T, pointer>;
  using typename Base::Wrapper;

  // TODO: iterator support
};

template <typename S, template <class> class F>
using Wrapper = WrapperGenerator<S, F>::Wrapper;

#endif // WRAPPER_H