#ifndef WRAPPER_H
#define WRAPPER_H

/**
 * This example is adapted from Oliver Rietmann's SoA wrapper approach:
 *     https://github.com/cern-nextgen/wp1.7-soa-wrapper/commit/21d5b849ef7336bd56ebd896c9f34339a5aa75e3
 * This version reduces code duplication by using C++26 reflection features.
 */

#include <experimental/meta>
#include <ranges>

namespace MemLayout {
using ptrdiff_t = decltype(static_cast<int *>(nullptr) - static_cast<int *>(nullptr));

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

template <class T>
using reference_restrict = T &__restrict__;
template <class T>
using const_reference_restrict = const T &__restrict__;
template <class T>
using pointer_restrict = T *__restrict__;
template <class T>
using const_pointer_restrict = const T *__restrict__;

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

//////////////// Operations on members

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
struct GetPointer {
  template <class... Args>
  constexpr SF operator()(Args &...args) const {
    return {&args...};
  }
  template <class... Args>
  constexpr SF operator()(const Args &...args) const {
    return {&args...};
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

template <class SF>
struct PreIncrement {
  template <class... Args>
  constexpr SF operator()(Args &...args) const {
    return {++args...};
  }
};

template <class SF>
struct PreDecrement {
  template <class... Args>
  constexpr SF operator()(Args &...args) const {
    return {--args...};
  }
};

template <class SF>
struct Advance {
  ptrdiff_t i;
  template <class... Args>
  constexpr SF operator()(const Args &...args) const {
    return {(args + i)...};
  }
};

struct CopyAssignment {
  template <class Left, class Right>
  constexpr Left &operator()(Left &left, const Right &right) const {
    return left = right;
  }
};

//////////////// Wrapper Generator
template <typename S, template <class> class F>
struct WrapperGeneratorBase {
  struct MembersImpl;

  consteval {
    if (^^F != ^^reference) {
      define_aggregate(^^MembersImpl, transform_members(^^S, [](std::meta::info type) {
        return substitute(^^F, {remove_cvref(type)});
      }));
    }
  }

  // The struct that contains the transformed members is either defined as MembersImpl or S, depending on whether
  // the transformation `F` is `reference`, which would be the identity transformation for the type `S`.
  using Members = std::conditional_t<is_enumerable_type(^^MembersImpl), MembersImpl, S>;

  struct Wrapper : public Members {
    using Generator = WrapperGeneratorBase<S, F>;

    template <class FunctionObject>
    auto apply(FunctionObject &&f) {
      auto construct_output = [&]<size_t... Is>(std::index_sequence<Is...>) {
        return f(this->[:nsdms(^^Members)[Is]:]...);
      };
      constexpr auto indices = std::make_index_sequence<count_members<Members>()>{};
      return construct_output(indices);
    }

    template <class FunctionObject>
    auto apply(FunctionObject &&f) const {
      auto construct_output = [&]<size_t... Is>(std::index_sequence<Is...>) {
        return f(this->[:nsdms(^^Members)[Is]:]...);
      };
      constexpr auto indices = std::make_index_sequence<count_members<Members>()>{};
      return construct_output(indices);
    }

    template <template <class> class OtherF, class FunctionObject>
    auto apply(const WrapperGeneratorBase<S, OtherF>::Wrapper &other, FunctionObject &&f) {
      auto construct_output = [&]<size_t... Is>(std::index_sequence<Is...>) -> S {
        return {f(this->[:nsdms(^^Members)[Is]:], other.[:nsdms(^^typename WrapperGeneratorBase<S, OtherF>::Members)[Is
        ]:])...};
      };
      constexpr auto indices = std::make_index_sequence<count_members<Members>()>{};
      return construct_output(indices);
    }

    template <template <class> class OtherF, class FunctionObject>
    auto apply(const WrapperGeneratorBase<S, OtherF>::Wrapper &other, FunctionObject &&f) const {
      auto construct_output = [&]<size_t... Is>(std::index_sequence<Is...>) -> S {
        return {f(this->[:nsdms(^^Members)[Is]:], other.[:nsdms(^^typename WrapperGeneratorBase<S, OtherF>::Members)[Is
        ]:])...};
      };
      constexpr auto indices = std::make_index_sequence<count_members<Members>()>{};
      return construct_output(indices);
    }

    //////// Random Access operators
    WrapperGeneratorBase<S, reference>::Wrapper operator[](size_t i) {
      return apply(RandomAccessAt<typename WrapperGeneratorBase<S, reference>::Wrapper>{i});
    }

    WrapperGeneratorBase<S, const_reference>::Wrapper operator[](size_t i) const {
      return apply(RandomAccessAt<typename WrapperGeneratorBase<S, const_reference>::Wrapper>{i});
    }

    ////// Constructors
    constexpr Wrapper() = default;
    constexpr Wrapper(Members m) : Members(m) {}

    template <typename... T>
    constexpr Wrapper(std::initializer_list<T>... args) : Members(args...) {}
    template <typename... T>
      requires std::constructible_from<Members, T &&...>
    constexpr Wrapper(T &&...t) : Members(std::forward<T>(t)...) {}

    ////// Conversion constructors
    template <typename T>
      requires requires {
        typename T::Generator;
        template_arguments_of(^^typename T::Generator)[0] == ^^S;
        template_arguments_of(^^typename T::Generator)[1] != ^^F;
      }
    constexpr Wrapper(T &other) : Wrapper(other.apply(AggregateConstructor<Wrapper>{})) {}

    template <typename T>
      requires requires {
        typename T::Generator;
        template_arguments_of(^^typename T::Generator)[0] == ^^S;
        template_arguments_of(^^typename T::Generator)[1] != ^^F;
      }
    constexpr Wrapper(const T &other) : Wrapper(other.apply(AggregateConstructor<Wrapper>{})) {}

    friend void swap(Wrapper a, Wrapper b) {
      using std::swap;
      [&]<size_t... Is>(std::index_sequence<Is...>) {
        (swap(a.[:nsdms(^^Members)[Is]:], b.[:nsdms(^^Members)[Is]:]), ...);
      }(std::make_index_sequence<count_members<Members>()>{});
    }
  };
};

// Generic wrapper generator
template <typename S, template <class> class F>
struct WrapperGenerator : public WrapperGeneratorBase<S, F> {
  using Base = WrapperGeneratorBase<S, F>;
  using BaseWrapper = typename Base::Wrapper;

  struct Wrapper : public BaseWrapper {
    using BaseWrapper::BaseWrapper; // inherit constructors
    using BaseWrapper::operator[];  // keep base overloads if needed
    constexpr Wrapper(BaseWrapper b) : BaseWrapper{static_cast<BaseWrapper &&>(b)} {}
  };
};

template <typename S>
struct WrapperGenerator<S, value> : public WrapperGeneratorBase<S, value> {
  using Base = WrapperGeneratorBase<S, value>;
  using BaseWrapper = typename Base::Wrapper;
  using Members = typename Base::Members;

  struct Wrapper : public BaseWrapper {
    using BaseWrapper::apply;
    using BaseWrapper::BaseWrapper; // inherit constructors

    friend bool operator<(const Wrapper &a, const Wrapper &b) {
      return a.[:nsdms(^^Members)[0]:] < b.[:nsdms(^^Members)[0]:];
    }
  };
};

template <typename S>
struct WrapperGenerator<S, reference> : public WrapperGeneratorBase<S, reference> {
  using Base = WrapperGeneratorBase<S, reference>;
  using BaseWrapper = typename Base::Wrapper;
  using Members = typename Base::Members;

  struct Wrapper : public BaseWrapper {
    using BaseWrapper::apply;

    constexpr Wrapper() = delete;
    constexpr Wrapper(BaseWrapper b) : BaseWrapper{static_cast<BaseWrapper &&>(b)} {}
    constexpr Wrapper(WrapperGenerator<S, value>::Wrapper &other)
        : BaseWrapper(other.apply(AggregateConstructor<BaseWrapper>{})) {}
    constexpr Wrapper(WrapperGenerator<S, reference_restrict>::Wrapper other)
        : BaseWrapper(other.apply(AggregateConstructor<BaseWrapper>{})) {}
    constexpr Wrapper(const Wrapper &other) = default;

    constexpr Wrapper &operator=(const WrapperGenerator<S, value>::Wrapper &other) {
      this->template apply<value>(other, CopyAssignment{});
      return *this;
    }
    constexpr Wrapper &operator=(const Wrapper &other) {
      this->template apply<reference>(other, CopyAssignment{});
      return *this;
    }
    constexpr Wrapper &operator=(const WrapperGenerator<S, const_reference>::Wrapper &other) {
      this->template apply<const_reference>(other, CopyAssignment{});
      return *this;
    }
    constexpr Wrapper &operator=(const WrapperGenerator<S, reference_restrict>::Wrapper &other) {
      this->template apply<reference_restrict>(other, CopyAssignment{});
      return *this;
    }
    constexpr Wrapper &operator=(const WrapperGenerator<S, const_reference_restrict>::Wrapper &other) {
      this->template apply<const_reference_restrict>(other, CopyAssignment{});
      return *this;
    }
    constexpr Wrapper(Wrapper &&other) = default;
    constexpr Wrapper &operator=(Wrapper &&other) { return operator=(other); }

    constexpr WrapperGenerator<S, pointer>::Wrapper operator&() {
      return apply(GetPointer<typename WrapperGenerator<S, pointer>::Wrapper>{});
    }
    constexpr pointer<WrapperGenerator<S, reference>::Wrapper> operator->() { return this; }

    friend bool operator<(const Wrapper &a, const Wrapper &b) {
      return a.[:nsdms(^^Members)[0]:] < b.[:nsdms(^^Members)[0]:];
    }
  };
};

template <typename S>
struct WrapperGenerator<S, const_reference> : public WrapperGeneratorBase<S, const_reference> {
  using Base = WrapperGeneratorBase<S, const_reference>;
  using BaseWrapper = typename Base::Wrapper;
  using Members = typename Base::Members;

  struct Wrapper : public BaseWrapper {
    using BaseWrapper::apply;

    constexpr Wrapper() = delete;
    constexpr Wrapper(BaseWrapper b) : BaseWrapper{static_cast<BaseWrapper &&>(b)} {}
    constexpr Wrapper(const WrapperGenerator<S, value>::Wrapper &other)
        : BaseWrapper(other.apply(AggregateConstructor<BaseWrapper>{})) {}
    constexpr Wrapper(const WrapperGenerator<S, reference>::Wrapper &other)
        : BaseWrapper(other.apply(AggregateConstructor<BaseWrapper>{})) {}
    constexpr Wrapper(const WrapperGenerator<S, reference_restrict>::Wrapper &other)
        : BaseWrapper(other.apply(AggregateConstructor<BaseWrapper>{})) {}
    constexpr Wrapper(const WrapperGenerator<S, const_reference_restrict>::Wrapper &other)
        : BaseWrapper(other.apply(AggregateConstructor<BaseWrapper>{})) {}

    // Why is this needed for std::sort to work??
    constexpr Wrapper operator=(const WrapperGenerator<S, reference>::Wrapper &other) { return Wrapper(other); }

    constexpr WrapperGenerator<S, const_pointer>::Wrapper operator&() const {
      return apply(GetPointer<typename WrapperGenerator<S, const_pointer>::Wrapper>{});
    }
    constexpr const_pointer<WrapperGenerator<S, const_reference>::Wrapper> operator->() const { return this; }

    friend bool operator<(const Wrapper &a, const Wrapper &b) {
      return a.[:nsdms(^^Members)[0]:] < b.[:nsdms(^^Members)[0]:];
    }
  };
};

template <typename S>
struct WrapperGenerator<S, pointer> : public WrapperGeneratorBase<S, pointer> {
  using Base = WrapperGeneratorBase<S, pointer>;
  using BaseWrapper = typename Base::Wrapper;
  using Members = typename Base::Members;

  struct Wrapper : public BaseWrapper {
    using BaseWrapper::BaseWrapper; // inherit constructors
    using BaseWrapper::operator[];  // keep base overloads if needed
    using BaseWrapper::apply;

    // Iterator traits for std::iterator_traits
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = ptrdiff_t;
    using value_type = typename WrapperGenerator<S, value>::Wrapper;
    using reference = typename WrapperGenerator<S, reference>::Wrapper;
    using pointer = void; // proxy iterator

    constexpr WrapperGenerator<S, MemLayout::reference>::Wrapper operator*() { return operator[](0); }
    constexpr WrapperGenerator<S, const_reference>::Wrapper operator*() const { return operator[](0); }
    constexpr WrapperGenerator<S, MemLayout::reference>::Wrapper operator->() { return operator[](0); }
    constexpr WrapperGenerator<S, const_reference>::Wrapper operator->() const { return operator[](0); }

    // ////// Comparison operators
    constexpr bool operator==(const Wrapper &other) const {
      return this->[:nsdms(^^Members)[0]:] == other.[:nsdms(^^Members)[0]:];
    }
    constexpr bool operator!=(const Wrapper &other) const { return !this->operator==(other); }
    constexpr bool operator<(const Wrapper &other) const {
      return this->[:nsdms(^^Members)[0]:] < other.[:nsdms(^^Members)[0]:];
    }
    constexpr bool operator<=(const Wrapper &other) const { return !(other < *this); }
    constexpr bool operator>(const Wrapper &other) const { return other < *this; }
    constexpr bool operator>=(const Wrapper &other) const { return !(*this < other); }

    // ////// Pointer arithmetic
    constexpr Wrapper operator+(ptrdiff_t i) const { return apply(Advance<Wrapper>{i}); }
    constexpr Wrapper operator-(ptrdiff_t i) const { return operator+(-i); }
    constexpr ptrdiff_t operator-(const Wrapper &other) const {
      return this->[:nsdms(^^Members)[0]:] - other.[:nsdms(^^Members)[0]:];
    }

    constexpr Wrapper &operator++() {
      apply(PreIncrement<Wrapper>{});
      return *this;
    }
    constexpr Wrapper &operator+=(ptrdiff_t i) { return *this = *this + i; }
    constexpr Wrapper &operator--() {
      apply(PreDecrement<Wrapper>{});
      return *this;
    }
    constexpr Wrapper &operator-=(ptrdiff_t i) { return *this = *this - i; }
  };
};

template <typename S>
struct WrapperGenerator<S, const_pointer> : public WrapperGeneratorBase<S, const_pointer> {
  using Base = WrapperGeneratorBase<S, const_pointer>;
  using BaseWrapper = typename Base::Wrapper;
  using Members = typename Base::Members;

  struct Wrapper : public BaseWrapper {
    using BaseWrapper::BaseWrapper; // inherit constructors
    using BaseWrapper::operator[];  // keep base overloads if needed
    using BaseWrapper::apply;

    constexpr WrapperGenerator<S, const_reference>::Wrapper operator*() const { return operator[](0); }
    constexpr WrapperGenerator<S, const_reference>::Wrapper operator->() const { return operator[](0); }

    ////// Comparison operators
    constexpr bool operator==(const Wrapper &other) const {
      return this->[:nsdms(^^Base)[0]:] == other.[:nsdms(^^Base)[0]:];
    }
    constexpr bool operator!=(const Wrapper &other) const { return !this->operator==(other); }
    constexpr bool operator<(const Wrapper &other) const {
      return this->[:nsdms(^^Base)[0]:] < other.[:nsdms(^^Base)[0]:];
    }

    ////// Pointer arithmetic
    constexpr Wrapper operator+(ptrdiff_t i) const { return apply(Advance<Wrapper>{i}); }
    constexpr Wrapper operator-(ptrdiff_t i) const { return operator+(-i); }
    constexpr ptrdiff_t operator-(const Wrapper &other) const {
      return this->[:nsdms(^^Members)[0]:] - other.[:nsdms(^^Members)[0]:];
    }

    constexpr Wrapper &operator++() {
      apply(PreIncrement<Wrapper>{});
      return *this;
    }
    constexpr Wrapper &operator+=(ptrdiff_t i) { return *this = *this + i; }
    constexpr Wrapper &operator--() {
      apply(PreDecrement<Wrapper>{});
      return *this;
    }
    constexpr Wrapper &operator-=(ptrdiff_t i) { return *this = *this - i; }
  };
};
} // namespace MemLayout

template <typename S, template <class> class F>
using Wrapper = MemLayout::WrapperGenerator<S, F>::Wrapper;

#endif // WRAPPER_H