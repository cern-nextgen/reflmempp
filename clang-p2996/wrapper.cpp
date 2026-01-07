/**
 * This example is adapted from Oliver Rietmann's SoA wrapper approach:
 *     https://github.com/cern-nextgen/wp1.7-soa-wrapper/commit/21d5b849ef7336bd56ebd896c9f34339a5aa75e3
 * This version reduces code duplication by using C++26 reflection features.
 */

#include <experimental/meta>
#include <iostream>
#include <ranges>
#include <span>
#include <utility>
#include <vector>

template <class T>
using value_type = T;

template <class T>
using reference = T &;

template <class T>
using const_reference = const T &;

////////////////

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

////////////////

template <typename S, template <class> class F>
struct WrapperGenerator {
  struct Base;

  consteval {
    define_aggregate(
        ^^Base, transform_members(^^S, [](std::meta::info type) { return substitute(^^F, {remove_cvref(type)}); }));
  }

  class Wrapper : public Base {

    template <typename Out, class FunctionObject>
    Out apply_to_members(Wrapper &s, FunctionObject &&f) {
      auto construct_output = [&]<size_t... Is>(std::index_sequence<Is...>) -> Out {
        return {f(s.[:nsdms(^^Base)[Is]:])...};
      };
      constexpr auto indices = std::make_index_sequence<count_members<Base>()>{};
      return construct_output(indices);
    }

  public:
    //////// Random Access operators

    S operator[](int i) { return apply_to_members<S>(static_cast<Wrapper &>(*this), evaluate_at{i}); }
    WrapperGenerator<S, F>::Wrapper operator[](int i) const {
      return apply_to_members<WrapperGenerator<S, F>::Wrapper>(static_cast<const Wrapper &>(*this), evaluate_at{i});
    }

    ////// Constructors

    template <typename... T>
    Wrapper(std::initializer_list<T>... args) : Base(args...) {}

    template <typename... T>
    Wrapper(T &&...t) : Base(std::forward<T>(t)...) {}

    Wrapper(const Base &b) : Base(b) {}

    ////// Conversion constructors
    template <typename T>
      requires(parent_of(^^T) != parent_of(^^Wrapper))
    Wrapper(T &other)
        : Wrapper([:expand_all(nsdms(type_of(bases_of(^^T, std::meta::access_context::unchecked())[0]))
                               ):] >> [&]<auto... members> { return Base{other.[:members:]...}; }) {}

    template <typename T>
      requires(parent_of(^^T) != parent_of(^^Wrapper))
    Wrapper(const T &other)
        : Wrapper([:expand_all(nsdms(type_of(bases_of(^^T, std::meta::access_context::unchecked())[0]))
                               ):] >> [&]<auto... members> { return Base{other.[:members:]...}; }) {}

  private:
    //////// Helper

    template <template <class> class F_out>
    struct cast {
      template <class T>
      F_out<T> operator()(F<T> &member) const {
        return member;
      }
    };

    struct evaluate_at {
      int i;
      template <class T>
      reference<T> operator()(F<T> &member) const {
        return member[i];
      }
      template <class T>
      const_reference<T> operator()(const F<T> &member) const {
        return member[i];
      }
    };
  };
};

template <typename S, template <class> class F>
using Wrapper = WrapperGenerator<S, F>::Wrapper;

///////////////////

struct Point {
  float &x;
  int &y;
  double &z;

  double sum() const { return x + y + z; };
};

using PointSoA = Wrapper<Point, std::vector>;
using PointVal = Wrapper<Point, value_type>;
using PointSpan = Wrapper<Point, std::span>;

int main() {
  PointVal p = {0.0f, 1, 2.0};
  PointSoA q = {{0.0f, 0.0f, 0.0f}, {1, 1, 1}, {2.0, 2.0, 2.0}};

  PointSpan s = q;
  s[1].x = 213445;

  q[0].z = 42;
  std::cout << "q[0] = { x: " << q[0].x << ", y: " << q[0].y << ", z: " << q[0].z << " }" << std::endl;
  std::cout << "q[1] = { x: " << q[1].x << ", y: " << q[1].y << ", z: " << q[1].z << " }" << std::endl;
  std::cout << "q[2] = { x: " << q[2].x << ", y: " << q[2].y << ", z: " << q[2].z << " }" << std::endl;
  std::cout << "sum q[2] == " << q[2].sum() << std::endl;

  std::cout << "q.y[1] = " << q.y[1] << std::endl;
  return 0;
}