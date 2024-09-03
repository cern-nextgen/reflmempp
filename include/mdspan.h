// Source: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107761

// HACK:
#define __cpp_lib_mdspan 202207L
#define __cpp_lib_submdspan 202306L

///////////////////////////////

// <mdspan> -*- C++ -*-

// Copyright (C) 2024 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

/** @file include/mdspan
 *  This is a Standard C++ Library header.
 */

#ifndef _GLIBCXX_MDSPAN
#define _GLIBCXX_MDSPAN 1

// #pragma GCC system_header

#define __glibcxx_want_mdspan
#define __glibcxx_want_submdspan
// #include <bits/version.h>

#ifdef __cpp_lib_mdspan // C++ >= 23

#include <concepts>
#include <ext/numeric_traits.h>
#include <span>
#include <type_traits>
#include <utility>

#ifdef __cpp_lib_submdspan // C++ >= 26
#include <tuple>
#endif

namespace std _GLIBCXX_VISIBILITY(default) {
  _GLIBCXX_BEGIN_NAMESPACE_VERSION

  // <
  // note: implementing 'concepts-only' for c++23
  // may be defined somewhere else other than <mdspan>
  template <typename> inline constexpr bool __enable_tuple_like = false;

#if __GNUC__ < 14
  template <typename _Tp>
  concept __tuple_like = __enable_tuple_like<remove_cvref_t<_Tp>>;

  template <typename _Tp>
  concept __pair_like = __tuple_like<_Tp> && tuple_size_v<remove_cvref_t<_Tp>>
  == 2;
#endif

  // specialization of __tuple_like for pair
  template <typename _T1, typename _T2> inline constexpr bool __enable_tuple_like<pair<_T1, _T2>> = true;

  // specialization of __tuple_like for tuple
  template <typename... _Ts> inline constexpr bool __enable_tuple_like<tuple<_Ts...>> = true;

  // specialization of __tuple_like for complex (c++26), ranges::subrange
  // will be defined somewhere else

  // >

  // <
  // note: adding '_Nth_type_t' after '_Nth_type' on <bits/utility.h>

  template <size_t _I, typename... _Ts> using _Nth_type_t = _Nth_type<_I, _Ts...>::type;

  // >

  namespace __mdspan {
  template <typename _Tp> struct _EmptyArray {
    // Indexing is undefined.
    __attribute__((__always_inline__, __noreturn__)) _Tp &operator[](size_t) const noexcept { __builtin_trap(); }

    static constexpr size_t size() noexcept { return 0; }
  };

  template <typename _Tp, size_t _Nm>
  using __maybe_empty_array = __conditional_t<_Nm != 0, array<_Tp, _Nm>, _EmptyArray<_Tp>>;

  template <size_t... _Extents> consteval auto __generate_dyn_idx_map() {
    constexpr size_t __rank = sizeof...(_Extents);
    __maybe_empty_array<size_t, __rank> __from_extents{_Extents...};
    __maybe_empty_array<size_t, __rank> __result;

    for (size_t __r = 0, __i = 0; __r < __rank; ++__r) {
      __result[__r] = __i;
      if (__from_extents[__r] == dynamic_extent)
        ++__i;
    }

    return __result;
  }

  template <size_t... _Extents> consteval auto __generate_dyn_idx_inv_map() {
    constexpr size_t __rank = sizeof...(_Extents);
    constexpr size_t __rank_dyn = ((_Extents == dynamic_extent) + ... + 0);
    __maybe_empty_array<size_t, __rank> __from_extents{_Extents...};
    __maybe_empty_array<size_t, __rank_dyn> __result;

    for (size_t __r = 0, __i = 0; __r < __rank; ++__r)
      if (__from_extents[__r] == dynamic_extent)
        __result[__i++] = __r;

    return __result;
  }

  template <typename _IndexType, typename _From> constexpr auto __index_cast(_From &&__from) noexcept {
    if constexpr (is_integral_v<_From> && !is_same_v<_From, bool>)
      return __from;
    else
      return static_cast<_IndexType>(__from);
  }

  // simplfy the use of integer packs with common patterns
  template <size_t _Nm, typename _F>
  __attribute__((always_inline)) constexpr decltype(auto) __apply_index_pack(_F &&__f) {
    return [&]<size_t... _Is>(index_sequence<_Is...>)->decltype(auto) {
      // expects to have a lambda as an argument
      return __f.template operator()<_Is...>();
    }
    (make_index_sequence<_Nm>{});
  }

  template <typename _To, typename _From> constexpr bool __is_repr_as(_From __val) { return in_range<_To>(__val); }

  template <typename _To, typename... _From> constexpr bool __are_repr_as(_From... __vals) {
    return (__mdspan::__is_repr_as<_To>(__vals) && ... && true);
  }

  template <typename _To, typename _From, size_t _Nm> constexpr bool __are_repr_as(span<_From, _Nm> __vals) {
    for (size_t __i = 0; __i < _Nm; ++__i)
      if (!__mdspan::__is_repr_as<_To>(__vals[__i]))
        return false;
    return true;
  }

  template <typename _IndexType, typename _From> constexpr bool __is_index_in_extent(_IndexType __ext, _From __val) {
    if constexpr (is_signed_v<_From>)
      if (__val < 0)
        return false;
    using _Tp = common_type_t<_IndexType, _From>;
    return static_cast<_Tp>(__val) < static_cast<_Tp>(__ext);
  }

  template <typename _Extents, typename... _From>
  constexpr bool __is_multidim_index_in(const _Extents &__ext, _From... __vals) {
    static_assert(_Extents::rank() == sizeof...(_From));
    return __mdspan::__apply_index_pack<_Extents::rank()>([&]<size_t... _Is> {
      return (__mdspan::__is_index_in_extent(__ext.extent(_Is), __vals) && ...);
    });
  }

  // fwd decl.
  template <typename _Mapping> constexpr typename _Mapping::index_type __mapping_offset(const _Mapping &);

#ifdef __cpp_lib_submdspan
  template <typename _Tp>
  concept __integral_constant_like =
      is_integral_v<_Tp> && !is_same_v<bool, remove_const_t<decltype(_Tp::value)>> &&
      convertible_to<_Tp, decltype(_Tp::value)> && equality_comparable_with<_Tp, decltype(_Tp::value)> &&
      bool_constant<_Tp() == _Tp::value>::value &&
      bool_constant<static_cast<decltype(_Tp::value)>(_Tp()) == _Tp::value>::value;

  // prerequisite of implementing submdspan
  // - implement c++23 tuple-like interactions
  template <typename _Tp, typename _IndexType>
  concept __index_pair_like = __pair_like<_Tp> && convertible_to<tuple_element_t<0, _Tp>, _IndexType> &&
      convertible_to<tuple_element_t<1, _Tp>, _IndexType>;

  template <typename _Tp> constexpr auto __de_ice(_Tp __val) {
    if constexpr (__mdspan::__integral_constant_like<_Tp>)
      return _Tp::value;
    else
      return __val;
  }

  template <typename... _Ts> __attribute__((always_inline)) constexpr void __for_each_types(auto &&__f) {
    // expects to have a lambda as an argument
    (__f.template operator()<_Ts>(), ...);
  }

  template <size_t _Nm, typename... _Ts>
  requires(sizeof...(_Ts) == _Nm)
      __attribute__((always_inline)) constexpr void __for_each_index_pack_and_types(auto &&__f) {
    [&]<size_t... _Is>(index_sequence<_Is...>) { (__f.template operator()<_Is, _Ts>(), ...); }
    (make_index_sequence<_Nm>{});
  }

  template <size_t _Nm, typename... _Args>
  requires(sizeof...(_Args) == _Nm)
      __attribute__((always_inline)) constexpr void __for_each_index_pack_and_args(auto &&__f, _Args... __args) {
    [&]<size_t... _Is>(index_sequence<_Is...>) { (__f.template operator()<_Is, _Args>(__args), ...); }
    (make_index_sequence<_Nm>{});
  }

  template <size_t _Idx, typename... _Args>
  __attribute__((always_inline)) constexpr decltype(auto) __get_element_at(_Args &&...__args) noexcept {
#ifdef __cpp_pack_indexing
    return std::forward<_Args...[_Idx]>(__args...[_Idx]);
#else
    return std::get<_Idx>(std::forward_as_tuple(std::forward<_Args>(__args)...));
#endif
  }

  // fwd decl.
  template <typename _Extents, typename _LayoutMapping, typename... _Slicers>
  constexpr auto __submdspan_mapping_impl(const _LayoutMapping &, _Slicers...);
#endif
  } // namespace __mdspan

  template <typename _IndexType, size_t... _Extents> class extents {
    static_assert(is_integral_v<_IndexType> && !is_same_v<_IndexType, bool>);
    static_assert(((__mdspan::__is_repr_as<_IndexType>(_Extents) || _Extents == dynamic_extent) && ...));

    struct _Impl {
      static constexpr size_t _S_rank = sizeof...(_Extents);
      static constexpr size_t _S_rank_dyn = ((_Extents == dynamic_extent) + ... + 0);
      static constexpr bool _S_req_size_is_always_zero = ((_Extents == 0) || ...);

      using _DynamicVals = __mdspan::__maybe_empty_array<_IndexType, _S_rank_dyn>;
      using _StaticVals = __mdspan::__maybe_empty_array<size_t, _S_rank>;

      static constexpr auto _S_dyn_idx_map = __mdspan::__generate_dyn_idx_map<_Extents...>();
      static constexpr auto _S_dyn_idx_inv_map = __mdspan::__generate_dyn_idx_inv_map<_Extents...>();

      static constexpr _StaticVals _S_static_vals{_Extents...};

      [[no_unique_address]] _DynamicVals _M_dyn_vals;

      template <size_t... _Is> static constexpr _DynamicVals _S_dyn_vals_zeros(index_sequence<_Is...>) noexcept {
        return _DynamicVals{((void)_Is, 0)...};
      }

      // ctor
      constexpr _Impl() noexcept requires(_S_rank_dyn == 0) = default;

      constexpr _Impl() noexcept requires(_S_rank_dyn != 0)
          : _M_dyn_vals(_S_dyn_vals_zeros(make_index_sequence<_S_rank_dyn>{})) {}

      template <typename... _DynVals>
      requires(sizeof...(_DynVals) == _S_rank_dyn) constexpr _Impl(_DynVals... __vals)
          : _M_dyn_vals{static_cast<_IndexType>(__vals)...} {}

      template <typename _Tp, size_t _Nm>
      requires(_Nm == _S_rank_dyn) constexpr _Impl(const span<_Tp, _Nm> &__vals) {
        if constexpr (_Nm > 0) {
          for (size_t __i = 0; __i < _Nm; ++__i)
            _M_dyn_vals[__i] = static_cast<_IndexType>(__vals[__i]);
        }
      }

      template <typename... _DynVals>
      requires(sizeof...(_DynVals) != _S_rank_dyn &&
               sizeof...(_DynVals) == _S_rank) constexpr _Impl(_DynVals... __vals) {
        __mdspan::__maybe_empty_array<_IndexType, _S_rank> __vals_list{static_cast<_IndexType>(__vals)...};

        for (size_t __i = 0; __i < _S_rank; ++__i) {
          auto __static_val = _S_static_vals[__i];
          if (__static_val == dynamic_extent)
            _M_dyn_vals[_S_dyn_idx_map[__i]] = __vals_list[__i];
          else
            __glibcxx_assert(__vals_list[__i] == static_cast<_IndexType>(__static_val));
        }
      }

      template <typename _Tp, size_t _Nm>
      requires(_Nm != _S_rank_dyn &&
               (_Nm == _S_rank || _Nm == dynamic_extent)) constexpr _Impl(const span<_Tp, _Nm> &__vals) {
        if constexpr (_Nm > 0) {
          for (size_t __i = 0; __i < _Nm; ++__i) {
            auto __static_val = _S_static_vals[__i];
            if (__static_val == dynamic_extent)
              _M_dyn_vals[_S_dyn_idx_map[__i]] = static_cast<_IndexType>(__vals[__i]);
            else
              __glibcxx_assert(static_cast<_IndexType>(__vals[__i]) == static_cast<_IndexType>(__static_val));
          }
        }
      }

      constexpr _Impl(const _DynamicVals &__vals) : _Impl(span(__vals)) {}

      constexpr _IndexType _M_extent(size_t __r) const noexcept {
        __glibcxx_assert(__r < _S_rank);
        return _S_static_vals[__r] == dynamic_extent ? _M_dyn_vals[_S_dyn_idx_map[__r]]
                                                     : static_cast<_IndexType>(_S_static_vals[__r]);
      }
    };

    [[no_unique_address]] _Impl _M_impl;

  public:
    using index_type = _IndexType;
    using size_type = make_unsigned_t<_IndexType>;
    using rank_type = size_t;

    // observers
    static constexpr rank_type rank() noexcept { return _Impl::_S_rank; }

    static constexpr rank_type rank_dynamic() noexcept { return _Impl::_S_rank_dyn; }

    static constexpr size_t static_extent(rank_type __r) noexcept {
      __glibcxx_assert(__r < rank());
      return _Impl::_S_static_vals[__r];
    }

    constexpr index_type extent(rank_type __r) const noexcept {
      __glibcxx_assert(__r < rank());
      return _M_impl._M_extent(__r);
    }

  private:
    // exposition-only observers
    constexpr index_type _M_fwd_prod_of_extents(rank_type __r) const noexcept {
      index_type __s = 1;
      for (rank_type __i = 0; __i < __r; ++__i)
        __s *= extent(__i);
      return __s;
    }

    constexpr index_type _M_rev_prod_of_extents(rank_type __r) const noexcept {
      index_type __s = 1;
      for (rank_type __i = rank() - 1; __i > __r; --__i)
        __s *= extent(__i);
      return __s;
    }

    constexpr index_type _M_size() const noexcept {
      if constexpr (_Impl::_S_req_size_is_always_zero)
        return 0;
      else
        return _M_fwd_prod_of_extents(rank());
    }

    friend struct layout_left;
    friend struct layout_right;
    friend struct layout_stride;

    template <typename _Mapping>
    friend constexpr typename _Mapping::index_type __mdspan::__mapping_offset(const _Mapping &);

    template <typename _Up, typename _OtherExtents, typename _OtherLayout, typename _OtherAccessor> friend class mdspan;

    template <typename _OtherExtents>
    static constexpr typename _Impl::_DynamicVals _S_x_vals_from_extents(const _OtherExtents &__other) noexcept {
      using _Result = _Impl::_DynamicVals;
      return __mdspan::__apply_index_pack<rank_dynamic()>([&]<size_t... _Idx> {
        return _Result{__other.extent(_Impl::_S_dyn_idx_inv_map[_Idx])...};
      });
    }

  public:
    // ctor
    constexpr extents() noexcept = default;

    template <typename... _OtherIndexTypes>
    requires(is_convertible_v<_OtherIndexTypes, index_type> &&...) &&
        (is_nothrow_constructible_v<index_type, _OtherIndexTypes> && ...) &&
        (sizeof...(_OtherIndexTypes) == rank() ||
         sizeof...(_OtherIndexTypes) == rank_dynamic()) constexpr explicit extents(_OtherIndexTypes... __exts) noexcept
        : _M_impl(static_cast<index_type>(__exts)...) {
      __glibcxx_assert(__mdspan::__are_repr_as<index_type>(__exts...));
    }

    template <typename _OtherIndexType, size_t _Nm>
    requires is_convertible_v<_OtherIndexType, index_type> && is_nothrow_constructible_v<index_type, _OtherIndexType> &&
        (_Nm == rank() || _Nm == rank_dynamic()) constexpr explicit(_Nm != rank_dynamic())
            extents(const span<_OtherIndexType, _Nm> &__exts) noexcept
        : _M_impl(__exts) {
      __glibcxx_assert(__mdspan::__are_repr_as<index_type>(__exts));
    }

    template <typename _OtherIndexType, size_t _Nm>
    requires is_convertible_v<_OtherIndexType, index_type> && is_nothrow_constructible_v<index_type, _OtherIndexType> &&
        (_Nm == rank() || _Nm == rank_dynamic()) constexpr explicit(_Nm != rank_dynamic())
            extents(const array<_OtherIndexType, _Nm> &__exts) noexcept
        : _M_impl(span(__exts)) {
      __glibcxx_assert(__mdspan::__are_repr_as<index_type>(span(__exts)));
    }

    template <typename _OtherIndexType, size_t... _OtherExtents>
    requires(sizeof...(_OtherExtents) == rank()) &&
        ((_OtherExtents == dynamic_extent || _Extents == dynamic_extent || _OtherExtents == _Extents) &&
         ...) constexpr explicit(((_Extents != dynamic_extent && _OtherExtents == dynamic_extent) || ...) ||
                                 (static_cast<make_unsigned_t<index_type>>(__gnu_cxx::__int_traits<index_type>::__max) <
                                  static_cast<make_unsigned_t<_OtherIndexType>>(
                                      __gnu_cxx::__int_traits<_OtherIndexType>::__max)))
            extents(const extents<_OtherIndexType, _OtherExtents...> &__other) noexcept
        : _M_impl(_S_x_vals_from_extents(__other)) {
      if constexpr (rank() > 0) {
        for (size_t __r = 0; __r < rank(); ++__r) {
          if constexpr (static_cast<make_unsigned_t<index_type>>(__gnu_cxx::__int_traits<index_type>::__max) <
                        static_cast<make_unsigned_t<_OtherIndexType>>(__gnu_cxx::__int_traits<_OtherIndexType>::__max))
            __glibcxx_assert(__mdspan::__is_repr_as<index_type>(__other.extent(__r)));
          __glibcxx_assert(static_extent(__r) == dynamic_extent ||
                           static_cast<index_type>(__other.extent(__r)) == static_cast<index_type>(static_extent(__r)));
        }
      }
    }

    template <typename _OtherIndexType, std::size_t... _OtherExtents>
    friend constexpr bool operator==(const extents &__lhs,
                                     const extents<_OtherIndexType, _OtherExtents...> &__rhs) noexcept {
      if constexpr (rank() != sizeof...(_OtherExtents)) {
        return false;
      } else {
        for (rank_type __r = 0; __r < rank(); ++__r) {
          using _CommonT = common_type_t<index_type, _OtherIndexType>;
          if (static_cast<_CommonT>(__lhs.extent(__r)) != static_cast<_CommonT>(__rhs.extent(__r)))
            return false;
        }
        return true;
      }
    }
  };

  template <typename... _IndexTypes>
  requires(is_convertible_v<_IndexTypes, size_t> && ...) extents(_IndexTypes...)
  ->extents<size_t, (_IndexTypes(0), dynamic_extent)...>;

  namespace __mdspan {
  template <typename _IndexType, size_t _Rank> consteval auto __dextents_impl() {
    return __mdspan::__apply_index_pack<_Rank>(
        []<size_t... _Is> { return extents<_IndexType, (_Is, dynamic_extent)...>{}; });
  }
  } // namespace __mdspan

  template <typename _IndexType, size_t _Rank>
  using dextents = decltype(__mdspan::__dextents_impl<_IndexType, _Rank>());

  struct layout_left {
    template <typename _Extents> class mapping;
  };

  struct layout_right {
    template <typename _Extents> class mapping;
  };

  struct layout_stride {
    template <typename _Extents> class mapping;
  };

  namespace __mdspan {
  template <typename> inline constexpr bool __is_extents = false;

  template <typename _IndexType, size_t... _Extents>
  inline constexpr bool __is_extents<extents<_IndexType, _Extents...>> = true;

  template <typename _Layout, typename _Mapping>
  concept __is_mapping_of = is_same_v<_Mapping, typename _Layout::template mapping<typename _Mapping::extents_type>>;

  template <typename _Mapping>
  concept __layout_mapping_alike = requires {
    requires __is_extents<typename _Mapping::extents_type>;
    { _Mapping::is_always_strided() } -> same_as<bool>;
    { _Mapping::is_always_exhaustive() } -> same_as<bool>;
    { _Mapping::is_always_unique() } -> same_as<bool>;
    bool_constant<_Mapping::is_always_strided()>::value;
    bool_constant<_Mapping::is_always_exhaustive()>::value;
    bool_constant<_Mapping::is_always_unique()>::value;
  };

  template <typename _Mapping> constexpr typename _Mapping::index_type __mapping_offset(const _Mapping &__m) {
    constexpr size_t __rank = _Mapping::extents_type::rank();
    if constexpr (__rank == 0)
      return __m();
    else if constexpr (_Mapping::extents_type::_Impl::_S_req_size_is_always_zero)
      return 0;
    else {
      if (__m.extents()._M_size() == 0)
        return 0;
      else
        return __mdspan::__apply_index_pack<__rank>([&]<size_t... _Idx> { return __m((_Idx, 0)...); });
    }
  }
  } // namespace __mdspan

  template <typename _Extents> class layout_left::mapping {
    static constexpr bool _S_req_span_size_is_repr(const _Extents &__ext) {
      using extents_type = _Extents;
      using index_type = _Extents::index_type;
      using rank_type = _Extents::rank_type;

      if constexpr (extents_type::rank() == 0)
        return true;

      index_type __prod = __ext.extent(0);
      for (rank_type __r = 1; __r < extents_type::rank(); ++__r)
        if (__builtin_mul_overflow(__prod, __ext.extent(__r), &__prod))
          return false;

      return true;
    }

  public:
    using extents_type = _Extents;
    using index_type = extents_type::index_type;
    using size_type = extents_type::size_type;
    using rank_type = extents_type::rank_type;
    using layout_type = layout_left;

    static_assert(__mdspan::__is_extents<_Extents>, "extents_type must be a specialization of std::extents");
    static_assert(extents_type::rank_dynamic() > 0 || _S_req_span_size_is_repr(extents_type()));

    constexpr mapping() noexcept = default;

    constexpr mapping(const mapping &) noexcept = default;

    constexpr mapping(const extents_type &__ext) noexcept : _M_extents(__ext) {
      __glibcxx_assert(_S_req_span_size_is_repr(__ext));
    }

    template <typename _OtherExtents>
    requires is_constructible_v<extents_type, _OtherExtents>
    constexpr explicit(!is_convertible_v<_OtherExtents, extents_type>)
        mapping(const mapping<_OtherExtents> &__other) noexcept
        : _M_extents(__other.extents()) {
      __glibcxx_assert(__mdspan::__is_repr_as<index_type>(__other.required_span_size()));
    }

    template <typename _OtherExtents>
    requires is_constructible_v<extents_type, _OtherExtents> &&
        (extents_type::rank() <= 1) constexpr explicit(!is_convertible_v<_OtherExtents, extents_type>)
            mapping(const layout_right::mapping<_OtherExtents> &__other) noexcept
        : _M_extents(__other.extents()) {
      __glibcxx_assert(__mdspan::__is_repr_as<index_type>(__other.required_span_size()));
    }

    template <typename _OtherExtents>
    requires is_constructible_v<extents_type, _OtherExtents>
    constexpr explicit(extents_type::rank() > 0) mapping(const layout_stride::mapping<_OtherExtents> &__other) noexcept
        : _M_extents(__other.extents()) {
      __glibcxx_assert(__mdspan::__is_repr_as<index_type>(__other.required_span_size()));
      if constexpr (extents_type::rank() > 0) {
        for (rank_type __r = 0; __r < extents_type::rank(); ++__r)
          __glibcxx_assert(__other.stride(__r) == __other.extents()._M_fwd_prod_of_extents(__r));
      }
    }

    constexpr mapping &operator=(const mapping &) noexcept = default;

    constexpr const extents_type &extents() const noexcept { return _M_extents; }

    constexpr index_type required_span_size() const noexcept { return _M_extents._M_size(); }

    template <typename... _Idx>
    requires(sizeof...(_Idx) == extents_type::rank()) && (is_convertible_v<_Idx, index_type> && ...) &&
        (is_nothrow_constructible_v<index_type, _Idx> && ...) constexpr index_type
        operator()(_Idx... __idx) const noexcept {
      __glibcxx_assert(__mdspan::__is_multidim_index_in(_M_extents, __mdspan::__index_cast<index_type>(__idx)...));
      return __mdspan::__apply_index_pack<extents_type::rank()>([&]<size_t... _P>->index_type {
        return ((static_cast<index_type>(__idx) * stride(_P)) + ... + 0);
      });
    }

    static constexpr bool is_always_unique() noexcept { return true; }

    static constexpr bool is_always_exhaustive() noexcept { return true; }

    static constexpr bool is_always_strided() noexcept { return true; }

    static constexpr bool is_unique() noexcept { return true; }

    static constexpr bool is_exhaustive() noexcept { return true; }

    static constexpr bool is_strided() noexcept { return true; }

    constexpr index_type stride(rank_type __r) const noexcept requires(extents_type::rank() > 0) {
      __glibcxx_assert(__r < extents_type::rank());
      return _M_extents._M_fwd_prod_of_extents(__r);
    }

    template <typename _OtherExtents>
    requires(_OtherExtents::rank() == extents_type::rank()) friend constexpr bool
    operator==(const mapping &__lhs, const mapping<_OtherExtents> &__rhs) noexcept {
      return __lhs.extents() == __rhs.extents();
    }

  private:
    [[no_unique_address]] extents_type _M_extents{};

#ifdef __cpp_lib_submdspan
    template <typename... _Slicers>
    friend constexpr auto submdspan_mapping(const mapping &__src, _Slicers... __slices) {
      return __mdspan::__submdspan_mapping_impl<_Extents>(__src, __slices...);
    }
#endif
  };

  template <typename _Extents> class layout_right::mapping {

    static constexpr bool _S_req_span_size_is_repr(const _Extents &__ext) {
      using extents_type = _Extents;
      using index_type = _Extents::index_type;
      using rank_type = _Extents::rank_type;

      if constexpr (extents_type::rank() == 0)
        return true;

      index_type __prod = __ext.extent(0);
      for (rank_type __r = 1; __r < extents_type::rank(); ++__r)
        if (__builtin_mul_overflow(__prod, __ext.extent(__r), &__prod))
          return false;

      return true;
    }

  public:
    using extents_type = _Extents;
    using index_type = extents_type::index_type;
    using size_type = extents_type::size_type;
    using rank_type = extents_type::rank_type;
    using layout_type = layout_right;

    static_assert(__mdspan::__is_extents<_Extents>, "extents_type must be a specialization of std::extents");
    static_assert(extents_type::rank_dynamic() > 0 || _S_req_span_size_is_repr(extents_type()));

    constexpr mapping() noexcept = default;

    constexpr mapping(const mapping &) noexcept = default;

    constexpr mapping(const extents_type &__ext) noexcept : _M_extents(__ext) {
      __glibcxx_assert(_S_req_span_size_is_repr(__ext));
    }

    template <typename _OtherExtents>
    requires is_constructible_v<extents_type, _OtherExtents>
    constexpr explicit(!is_convertible_v<_OtherExtents, extents_type>)
        mapping(const mapping<_OtherExtents> &__other) noexcept
        : _M_extents(__other.extents()) {
      __glibcxx_assert(__mdspan::__is_repr_as<index_type>(__other.required_span_size()));
    }

    template <typename _OtherExtents>
    requires is_constructible_v<extents_type, _OtherExtents> &&
        (extents_type::rank() <= 1) constexpr explicit(!is_convertible_v<_OtherExtents, extents_type>)
            mapping(const layout_left::mapping<_OtherExtents> &__other) noexcept
        : _M_extents(__other.extents()) {
      __glibcxx_assert(__mdspan::__is_repr_as<index_type>(__other.required_span_size()));
    }

    template <typename _OtherExtents>
    requires is_constructible_v<extents_type, _OtherExtents>
    constexpr explicit(extents_type::rank() > 0) mapping(const layout_stride::mapping<_OtherExtents> &__other) noexcept
        : _M_extents(__other.extents()) {
      __glibcxx_assert(__mdspan::__is_repr_as<index_type>(__other.required_span_size()));
      if constexpr (extents_type::rank() > 0) {
        for (rank_type __r = 0; __r < extents_type::rank(); ++__r)
          __glibcxx_assert(__other.stride(__r) == __other.extents()._M_rev_prod_of_extents(__r));
      }
    }

    constexpr mapping &operator=(const mapping &) noexcept = default;

    constexpr const extents_type &extents() const noexcept { return _M_extents; }

    constexpr index_type required_span_size() const noexcept { return _M_extents._M_size(); }

    template <typename... _Idx>
    requires(sizeof...(_Idx) == extents_type::rank()) && (is_convertible_v<_Idx, index_type> && ...) &&
        (is_nothrow_constructible_v<index_type, _Idx> && ...) constexpr index_type
        operator()(_Idx... __idx) const noexcept {
      __glibcxx_assert(__mdspan::__is_multidim_index_in(_M_extents, __mdspan::__index_cast<index_type>(__idx)...));
      return __mdspan::__apply_index_pack<extents_type::rank()>([&]<size_t... _P>->index_type {
        return ((static_cast<index_type>(__idx) * stride(_P)) + ... + 0);
      });
    }

    static constexpr bool is_always_unique() noexcept { return true; }

    static constexpr bool is_always_exhaustive() noexcept { return true; }

    static constexpr bool is_always_strided() noexcept { return true; }

    static constexpr bool is_unique() noexcept { return true; }

    static constexpr bool is_exhaustive() noexcept { return true; }

    static constexpr bool is_strided() noexcept { return true; }

    constexpr index_type stride(rank_type __r) const noexcept requires(extents_type::rank() > 0) {
      __glibcxx_assert(__r < extents_type::rank());
      return _M_extents._M_rev_prod_of_extents(__r);
    }

    template <typename _OtherExtents>
    requires(_OtherExtents::rank() == extents_type::rank()) friend constexpr bool
    operator==(const mapping &__lhs, const mapping<_OtherExtents> &__rhs) noexcept {
      return __lhs.extents() == __rhs.extents();
    }

  private:
    [[no_unique_address]] extents_type _M_extents{};

#ifdef __cpp_lib_submdspan
    template <typename... _Slicers>
    friend constexpr auto submdspan_mapping(const mapping &__src, _Slicers... __slices) {
      return __mdspan::__submdspan_mapping_impl<_Extents>(__src, __slices...);
    }
#endif
  };

  template <typename _Extents> class layout_stride::mapping {

    static constexpr bool _S_req_span_size_is_repr(const _Extents &__ext) {
      using extents_type = _Extents;
      using index_type = _Extents::index_type;
      using rank_type = _Extents::rank_type;

      if constexpr (extents_type::rank() == 0)
        return true;

      index_type __prod = __ext.extent(0);
      for (rank_type __r = 1; __r < extents_type::rank(); ++__r)
        if (__builtin_mul_overflow(__prod, __ext.extent(__r), &__prod))
          return false;

      return true;
    }

  public:
    using extents_type = _Extents;
    using index_type = extents_type::index_type;
    using size_type = extents_type::size_type;
    using rank_type = extents_type::rank_type;
    using layout_type = layout_stride;

    static_assert(__mdspan::__is_extents<_Extents>, "extents_type must be a specialization of std::extents");
    static_assert(extents_type::rank_dynamic() > 0 || _S_req_span_size_is_repr(extents_type()));

  private:
    static constexpr rank_type _S_rank = extents_type::rank();
    using _StrideStorage = __mdspan::__maybe_empty_array<index_type, _S_rank>;

    template <typename _OtherMapping> static constexpr _StrideStorage _S_fill_strides(const _OtherMapping &__m) {
      return __mdspan::__apply_index_pack<_S_rank>([&]<size_t... _Idx> {
        return _StrideStorage{static_cast<index_type>(__m.stride(_Idx))...};
      });
    }

    static constexpr const _StrideStorage &_S_fill_strides(const _StrideStorage &__strides) { return __strides; }

    template <typename _OtherIndexType>
    static constexpr _StrideStorage _S_fill_strides(span<_OtherIndexType, _S_rank> __strides) {
      return __mdspan::__apply_index_pack<_S_rank>([&]<size_t... _Idx> {
        return _StrideStorage{static_cast<index_type>(__strides[_Idx])...};
      });
    }

    template <typename _OtherIndexType, size_t _Nm>
    static constexpr index_type _S_req_span_size(const extents_type &__e, span<_OtherIndexType, _Nm> __strides) {
      if constexpr (_S_rank == 0)
        return 1;
      else if constexpr (extents_type::_Impl::_S_req_size_is_always_zero)
        return 0;
      else {
        if (__e._M_size() == 0)
          return 0;
        else
          return __mdspan::__apply_index_pack<_S_rank>([&]<size_t... _Idx> {
            return 1 + (((__e.extent(_Idx) - 1) * __mdspan::__index_cast<index_type>(__strides[_Idx])) + ...);
          });
      }
    }

    static constexpr index_type _S_req_span_size(const extents_type &__e, const _StrideStorage &__strides) {
      return _S_req_span_size(__e, span(__strides));
    }

    constexpr auto _M_permutated_by_strides() const noexcept requires(_S_rank > 1) {
      array<size_t, _S_rank> __permute =
          __mdspan::__apply_index_pack<_S_rank>([]<size_t... _Idx> { return array<size_t, _S_rank>{_Idx...}; });
      for (rank_type __i = _S_rank - 1; __i > 0; --__i) {
        for (rank_type __r = 0; __r < __i; ++__r) {
          if (_M_strides[__permute[__r]] > _M_strides[__permute[__r + 1]])
            std::swap(__permute[__r], __permute[__r + 1]);
          else {
            if (_M_strides[__permute[__r]] == _M_strides[__permute[__r + 1]] && _M_extents.extent(__permute[__r]) > 1)
              std::swap(__permute[__r], __permute[__r + 1]);
          }
        }
      }
      return __permute;
    }

  public:
    constexpr mapping() noexcept
        : _M_extents(extents_type()), _M_strides(_S_fill_strides(layout_right::mapping<extents_type>())) {
      __glibcxx_assert(__mdspan::__is_repr_as<index_type>(layout_right::mapping<extents_type>().required_span_size()));
    }

    constexpr mapping(const mapping &) noexcept = default;

    template <typename _OtherIndexType>
    requires is_convertible_v<const _OtherIndexType &, index_type> &&
        is_nothrow_constructible_v<index_type, const _OtherIndexType &>
    constexpr mapping(const extents_type &__e, span<_OtherIndexType, _S_rank> __strides) noexcept
        : _M_extents(__e), _M_strides(_S_fill_strides(__strides)) {
      if constexpr (_S_rank > 0) {
        for (rank_type __r = 0; __r < _S_rank; ++__r)
          __glibcxx_assert(__strides[__r] > 0);
        __glibcxx_assert(__mdspan::__is_repr_as<index_type>(_S_req_span_size(__e, __strides)));
        if constexpr (_S_rank > 1) {
          const auto __permute = _M_permutated_by_strides();
          for (rank_type __r = 1; __r < _S_rank; ++__r)
            __glibcxx_assert(__strides[__permute[__r]] >=
                             __strides[__permute[__r - 1]] * __e.extent(__permute[__r - 1]));
        }
      }
    }

    template <typename _OtherIndexType>
    requires is_convertible_v<const _OtherIndexType &, index_type> &&
        is_nothrow_constructible_v<index_type, const _OtherIndexType &>
    constexpr mapping(const extents_type &__e, const array<_OtherIndexType, _S_rank> &__strides) noexcept
        : mapping(__e, span(__strides)) {}

    template <__mdspan::__layout_mapping_alike _StridedLayoutMapping>
    requires is_constructible_v<extents_type, typename _StridedLayoutMapping::extents_type> &&
        (_StridedLayoutMapping::is_always_unique() && _StridedLayoutMapping::is_always_strided()) constexpr explicit(
            !(is_convertible_v<typename _StridedLayoutMapping::extents_type, extents_type> &&
              (__mdspan::__is_mapping_of<layout_left, _StridedLayoutMapping> ||
               __mdspan::__is_mapping_of<layout_right, _StridedLayoutMapping> ||
               __mdspan::__is_mapping_of<layout_stride, _StridedLayoutMapping>)))
            mapping(const _StridedLayoutMapping &__other) noexcept
        : _M_extents(__other.extents()), _M_strides(_S_fill_strides(__other)) {
      if constexpr (_S_rank > 0) {
        for (rank_type __r = 0; __r < _S_rank; ++__r)
          __glibcxx_assert(__other.stride(__r) > 0);
        __glibcxx_assert(__mdspan::__is_repr_as<index_type>(__other.required_span_size()));
        __glibcxx_assert(__mdspan::__mapping_offset(__other) == 0);
      }
    }

    constexpr mapping &operator=(const mapping &) noexcept = default;

    constexpr const extents_type &extents() const noexcept { return _M_extents; }

    constexpr _StrideStorage strides() const noexcept { return _M_strides; }

    constexpr index_type required_span_size() const noexcept { return _S_req_span_size(_M_extents, _M_strides); }

    template <typename... _Idx>
    requires(sizeof...(_Idx) == _S_rank) && (is_convertible_v<_Idx, index_type> && ...) &&
        (is_nothrow_constructible_v<index_type, _Idx> && ...) constexpr index_type
        operator()(_Idx... __idx) const noexcept {
      __glibcxx_assert(__mdspan::__is_multidim_index_in(_M_extents, __mdspan::__index_cast<index_type>(__idx)...));
      return __mdspan::__apply_index_pack<_S_rank>([&]<size_t... _P>->index_type {
        return ((static_cast<index_type>(__idx) * stride(_P)) + ... + 0);
      });
    }

    static constexpr bool is_always_unique() noexcept { return true; }

    static constexpr bool is_always_exhaustive() noexcept { return false; }

    static constexpr bool is_always_strided() noexcept { return true; }

    static constexpr bool is_unique() noexcept { return true; }

    constexpr bool is_exhaustive() const noexcept {
      if constexpr (_S_rank == 0)
        return true;
      else {
        const index_type __span_size = required_span_size();
        if (__span_size != 0)
          return __span_size == _M_extents._M_size();
        else {
          if constexpr (_S_rank == 1)
            return _M_strides[0] == 1;
          else {
            rank_type __largest_at_r = 0;
            for (rank_type __r = 1; __r < _S_rank; ++__r) {
              if (_M_strides[__r] > _M_strides[__largest_at_r])
                __largest_at_r = __r;
            }
            for (rank_type __r = 0; __r < _S_rank; ++__r) {
              if (_M_extents.extent(__r) == 0 && __r != __largest_at_r)
                return false;
            }
            return true;
          }
        }
      }
    }

    static constexpr bool is_strided() noexcept { return true; }

    constexpr index_type stride(rank_type __r) const noexcept requires(_S_rank > 0) {
      __glibcxx_assert(__r < _S_rank);
      return _M_strides[__r];
    }

    template <__mdspan::__layout_mapping_alike _OtherMapping>
    requires(_OtherMapping::extents_type::rank() == _S_rank) &&
        (_OtherMapping::is_always_strided()) friend constexpr bool operator==(const mapping &__lhs,
                                                                              const _OtherMapping &__rhs) noexcept {
      if (__lhs.extents() != __rhs.extents() || __mdspan::__mapping_offset(__rhs) != 0)
        return false;
      for (rank_type __r = 0; __r < _S_rank; ++__r)
        if (__lhs.stride(__r) != __rhs.stride(__r))
          return false;
      return true;
    }

  private:
    [[no_unique_address]] extents_type _M_extents;
    [[no_unique_address]] _StrideStorage _M_strides;

#ifdef __cpp_lib_submdspan
    template <typename... _Slicers>
    friend constexpr auto submdspan_mapping(const mapping &__src, _Slicers... __slices) {
      return __mdspan::__submdspan_mapping_impl<_Extents>(__src, __slices...);
    }
#endif
  };

  template <typename _Tp> struct default_accessor {
    using offset_policy = default_accessor;
    using element_type = _Tp;
    using reference = element_type &;
    using data_handle_type = element_type *;

    static_assert(sizeof(_Tp) > 0, "element_type must be a complete type");
    static_assert(!is_abstract_v<_Tp>, "element_type cannot be an abstract type");
    static_assert(!is_array_v<_Tp>, "element_type cannot be an array type");

    constexpr default_accessor() noexcept = default;

    template <typename _Up>
    requires is_convertible_v<_Up (*)[], _Tp (*)[]>
    constexpr default_accessor(default_accessor<_Up>) noexcept {}

    constexpr reference access(data_handle_type __p, size_t __i) const noexcept { return __p[__i]; }

    constexpr data_handle_type offset(data_handle_type __p, size_t __i) const noexcept { return __p + __i; }
  };

  template <typename _Tp, typename _Extents, typename _LayoutPolicy = layout_right,
            typename _AccessorPolicy = default_accessor<_Tp>>
  class mdspan {
  public:
    using extents_type = _Extents;
    using layout_type = _LayoutPolicy;
    using accessor_type = _AccessorPolicy;
    using mapping_type = layout_type::template mapping<extents_type>;
    using element_type = _Tp;
    using value_type = remove_cv_t<_Tp>;
    using index_type = extents_type::index_type;
    using size_type = extents_type::size_type;
    using rank_type = extents_type::rank_type;
    using data_handle_type = accessor_type::data_handle_type;
    using reference = accessor_type::reference;

    static_assert(sizeof(_Tp) > 0, "element_type must be a complete type");
    static_assert(!is_abstract_v<_Tp>, "element_type cannot be an abstract type");
    static_assert(!is_array_v<_Tp>, "element_type cannot be an array type");
    static_assert(__mdspan::__is_extents<_Extents>, "extents_type must be a specialization of std::extents");
    static_assert(is_same_v<_Tp, typename _AccessorPolicy::element_type>);

    static constexpr rank_type rank() noexcept { return extents_type::rank(); }

    static constexpr rank_type rank_dynamic() noexcept { return extents_type::rank_dynamic(); }

    static constexpr size_t static_extent(rank_type __r) noexcept { return extents_type::static_extent(__r); }

    constexpr index_type extent(rank_type __r) const noexcept { return extents().extent(__r); }

  private:
    [[no_unique_address]] accessor_type _M_acc;
    [[no_unique_address]] mapping_type _M_map;
    [[no_unique_address]] data_handle_type _M_ptr;

    template <typename _Up, typename _OtherExtents, typename _OtherLayoutPolicy, typename _OtherAccessor>
    friend class mdspan;

  public:
    constexpr mdspan() requires(rank_dynamic() > 0) &&
        is_default_constructible_v<data_handle_type> &&is_default_constructible_v<mapping_type>
            &&is_default_constructible_v<accessor_type> : _M_acc(),
        _M_map(), _M_ptr() {}

    constexpr mdspan(const mdspan &) = default;

    constexpr mdspan(mdspan &&) = default;

    template <typename... _OtherIndexTypes>
    requires(is_convertible_v<_OtherIndexTypes, index_type> &&...) &&
        (is_nothrow_constructible_v<index_type, _OtherIndexTypes> && ...) &&
        (sizeof...(_OtherIndexTypes) == rank() || sizeof...(_OtherIndexTypes) == rank_dynamic()) &&
        is_constructible_v<mapping_type, extents_type>
            &&is_default_constructible_v<accessor_type> constexpr explicit mdspan(data_handle_type __p,
                                                                                  _OtherIndexTypes... __exts)
        : _M_acc(),
    _M_map(extents_type(static_cast<index_type>(std::move(__exts))...)), _M_ptr(std::move(__p)) {}

    template <typename _OtherIndexType, size_t _Nm>
    requires is_convertible_v<const _OtherIndexType &, index_type> &&
        is_nothrow_constructible_v<index_type, const _OtherIndexType &> &&(_Nm == rank() || _Nm == rank_dynamic()) &&
        is_constructible_v<mapping_type, extents_type> &&is_default_constructible_v<accessor_type> constexpr explicit(
            _Nm != rank_dynamic()) mdspan(data_handle_type __p, std::span<_OtherIndexType, _Nm> __exts)
        : _M_acc(),
    _M_map(extents_type(__exts)), _M_ptr(std::move(__p)) {}

    template <typename _OtherIndexType, size_t _Nm>
    requires is_convertible_v<const _OtherIndexType &, index_type> &&
        is_nothrow_constructible_v<index_type, const _OtherIndexType &> &&(_Nm == rank() || _Nm == rank_dynamic()) &&
        is_constructible_v<mapping_type, extents_type> &&is_default_constructible_v<accessor_type> constexpr explicit(
            _Nm != rank_dynamic()) mdspan(data_handle_type __p, const std::array<_OtherIndexType, _Nm> &__exts)
        : _M_acc(),
    _M_map(extents_type(__exts)), _M_ptr(std::move(__p)) {}

    constexpr mdspan(data_handle_type __p, const extents_type &__ext) requires
        is_constructible_v<mapping_type, const extents_type &> && is_default_constructible_v<accessor_type>
        : _M_acc(), _M_map(__ext), _M_ptr(std::move(__p)) {}

    constexpr mdspan(data_handle_type __p, const mapping_type &__m) requires is_default_constructible_v<accessor_type>
        : _M_acc(), _M_map(__m), _M_ptr(std::move(__p)) {}

    constexpr mdspan(data_handle_type __p, const mapping_type &__m, const accessor_type &__a)
        : _M_acc(__a), _M_map(__m), _M_ptr(std::move(__p)) {}

    template <typename _Up, typename _OtherExtents, typename _OtherLayoutPolicy, typename _OtherAccessor>
    requires is_constructible_v < mapping_type, const typename _OtherLayoutPolicy::template mapping<_OtherExtents>
    & > &&is_constructible_v<accessor_type, const _OtherAccessor &> &&is_constructible_v<
            data_handle_type, const typename _OtherAccessor::data_handle_type &> &&
            is_constructible_v<extents_type, _OtherExtents> constexpr explicit(
                !is_convertible_v<const typename _OtherLayoutPolicy::template mapping<_OtherExtents> &, mapping_type> ||
                !is_convertible_v<const _OtherAccessor &, accessor_type>)
                mdspan(const mdspan<_Up, _OtherExtents, _OtherLayoutPolicy, _OtherAccessor> &__other)
        : _M_acc(__other._M_acc),
    _M_map(__other._M_map), _M_ptr(__other._M_ptr) {
      if constexpr (rank() > 0) {
        for (size_t __r = 0; __r < rank(); ++__r)
          __glibcxx_assert(static_extent(__r) == dynamic_extent ||
                           static_cast<index_type>(__other.extent(__r)) == static_cast<index_type>(static_extent(__r)));
      }
    }

    constexpr mdspan &operator=(const mdspan &) = default;

    constexpr mdspan &operator=(mdspan &&) = default;

    // element access
    template <typename... _Idx>
    requires(is_convertible_v<_Idx, index_type> &&...) && (is_nothrow_constructible_v<index_type, _Idx> && ...) &&
        (sizeof...(_Idx) == rank()) constexpr reference operator[](_Idx... __idx) const {
      __glibcxx_assert(__mdspan::__is_multidim_index_in(extents(), __mdspan::__index_cast<index_type>(__idx)...));
      return _M_acc.access(_M_ptr, _M_map(static_cast<index_type>(std::move(__idx))...));
    }

    template <typename _OtherIndexType> constexpr reference operator[](span<_OtherIndexType, rank()> __idx) const {
      return __mdspan::__apply_index_pack<rank()>([&]<size_t... _Is>->reference {
        return operator[](static_cast<const _OtherIndexType &>(__idx[_Is])...);
      });
    }

    template <typename _OtherIndexType>
    constexpr reference operator[](const array<_OtherIndexType, rank()> &__idx) const {
      return operator[](span(__idx));
    }

    // observers
    constexpr size_type size() const noexcept { return _M_map.extents()._M_size(); }

    [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }

    friend constexpr void swap(mdspan &__lhs, mdspan &__rhs) noexcept {
      std::swap(__lhs._M_acc, __rhs._M_acc);
      std::swap(__lhs._M_map, __rhs._M_map);
      std::swap(__lhs._M_ptr, __rhs._M_ptr);
    }

    constexpr const extents_type &extents() const noexcept { return _M_map.extents(); }

    constexpr const data_handle_type &data_handle() const noexcept { return _M_ptr; }

    constexpr const mapping_type &mapping() const noexcept { return _M_map; }

    constexpr const accessor_type &accessor() const noexcept { return _M_acc; }

    static constexpr bool is_always_unique() noexcept { return mapping_type::is_always_unique(); }

    static constexpr bool is_always_exhaustive() noexcept { return mapping_type::is_always_exhaustive(); }

    static constexpr bool is_always_strided() noexcept { return mapping_type::is_always_strided(); }

    constexpr bool is_unique() const noexcept { return _M_map.is_unique(); }

    constexpr bool is_exhaustive() const noexcept { return _M_map.is_exhaustive(); }

    constexpr bool is_strided() const noexcept { return _M_map.is_strided(); }

    constexpr index_type stride(rank_type __r) const { return _M_map.stride(__r); }
  };

  template <typename _CArray>
  requires is_array_v<_CArray> &&(rank_v<_CArray> == 1) mdspan(_CArray &)
      ->mdspan<remove_all_extents_t<_CArray>, extents<size_t, extent_v<_CArray, 0>>>;

  template <typename _P>
  requires is_pointer_v<remove_reference_t<_P>> mdspan(_P &&)
  ->mdspan<remove_pointer_t<remove_reference_t<_P>>, extents<size_t>>;

  template <typename _Tp, typename... _Extents>
  requires(is_convertible_v<_Extents, size_t> && ...) &&
      (sizeof...(_Extents) > 0) explicit mdspan(_Tp *, _Extents...)->mdspan<_Tp, dextents<size_t, sizeof...(_Extents)>>;

  template <typename _Tp, typename _OtherIndexType, size_t _Nm>
  mdspan(_Tp *, span<_OtherIndexType, _Nm>) -> mdspan<_Tp, dextents<size_t, _Nm>>;

  template <typename _Tp, typename _OtherIndexType, size_t _Nm>
  mdspan(_Tp *, const array<_OtherIndexType, _Nm> &) -> mdspan<_Tp, dextents<size_t, _Nm>>;

  template <typename _Tp, typename _IndexType, size_t... _Extents>
  mdspan(_Tp *, const extents<_IndexType, _Extents...> &) -> mdspan<_Tp, extents<_IndexType, _Extents...>>;

  template <typename _Tp, typename _Mapping>
  mdspan(_Tp *, const _Mapping &) -> mdspan<_Tp, typename _Mapping::extents_type, typename _Mapping::layout_type>;

  template <typename _Mapping, typename _Accessor>
  mdspan(const typename _Accessor::data_handle_type &, const _Mapping &, const _Accessor &)
      -> mdspan<typename _Accessor::element_type, typename _Mapping::extents_type, typename _Mapping::layout_type,
                _Accessor>;

#ifdef __cpp_lib_submdspan // C++ >= 26

  template <typename _Offset, typename _Extent, typename _Stride> struct strided_slice {
    using offset_type = _Offset;
    using extent_type = _Extent;
    using stride_type = _Stride;

    [[no_unique_address]] offset_type offset{};
    [[no_unique_address]] extent_type extent{};
    [[no_unique_address]] stride_type stride{};

    static_assert(is_integral_v<_Offset> || __mdspan::__integral_constant_like<_Offset>);
    static_assert(is_integral_v<_Extent> || __mdspan::__integral_constant_like<_Extent>);
    static_assert(is_integral_v<_Stride> || __mdspan::__integral_constant_like<_Stride>);
  };

  template <typename _LayoutMapping> struct submdspan_mapping_result {
    [[no_unique_address]] _LayoutMapping mapping = _LayoutMapping();
    size_t offset;
  };

  struct full_extent_t {
    explicit full_extent_t() = default;
  };
  inline constexpr full_extent_t full_extent{};

  namespace __mdspan {
  template <typename> inline constexpr bool __is_strided_slice = false;

  template <typename _Offset, typename _Extent, typename _Stride>
  inline constexpr bool __is_strided_slice<strided_slice<_Offset, _Extent, _Stride>> = true;

  template <typename _Slice, typename _IndexType = size_t>
  concept __slice_specifier = convertible_to<_Slice, _IndexType> || __mdspan::__index_pair_like<_Slice, _IndexType> ||
      is_convertible_v<_Slice, full_extent_t> || __mdspan::__is_strided_slice<_Slice>;

  template <typename _IndexType, size_t _Kp, typename... _Slicers>
  constexpr _IndexType __first_of_extent_slice(_Slicers... __slices) {
    static_assert(is_integral_v<_IndexType>);
    using _Slicer = _Nth_type_t<_Kp, _Slicers...>;
    auto &__slice = __mdspan::__get_element_at<_Kp>(__slices...);
    if constexpr (convertible_to<_Slicer, _IndexType>)
      return __mdspan::__index_cast<_IndexType>(__slice);
    else if constexpr (__mdspan::__index_pair_like<_Slicer, _IndexType>)
      return __mdspan::__index_cast<_IndexType>(std::get<0>(__slice));
    else if constexpr (__mdspan::__is_strided_slice<_Slicer>)
      return __mdspan::__index_cast<_IndexType>(__mdspan::__de_ice(__slice.offset));
    else
      return 0;
  }

  template <size_t _Kp, typename _Extents, typename... _Slicers>
  constexpr auto __last_of_extent_slice(const _Extents &__src, _Slicers... __slices) {
    static_assert(__mdspan::__is_extents<_Extents>);
    using _IndexType = _Extents::index_type;
    using _Slicer = _Nth_type_t<_Kp, _Slicers...>;
    auto &__slice = __mdspan::__get_element_at<_Kp>(__slices...);
    if constexpr (convertible_to<_Slicer, _IndexType>)
      return __mdspan::__index_cast<_IndexType>(__mdspan::__de_ice(__slice) + 1);
    else if constexpr (__mdspan::__index_pair_like<_Slicer, _IndexType>)
      return __mdspan::__index_cast<_IndexType>(std::get<1>(__slice));
    else if constexpr (__mdspan::__is_strided_slice<_Slicer>)
      return __mdspan::__index_cast<_IndexType>(__mdspan::__de_ice(__slice.offset) +
                                                __mdspan::__de_ice(__slice.extent));
    else
      return __mdspan::__index_cast<_IndexType>(__src.extent(_Kp));
  }

  template <typename _IndexType, typename... _Slicers> consteval auto __generate_map_ranks() {
    constexpr size_t __rank = sizeof...(_Slicers);
    array<size_t, __rank> __result;
    __mdspan::__for_each_types<_Slicers...>([&, __k = 0, __j = 0]<typename _Slice> mutable {
      if constexpr (convertible_to<_Slice, _IndexType>)
        __result[__k] = dynamic_extent;
      else
        __result[__k] = __j++;
      ++__k;
    });
    return __result;
  }

  // TODO: add '__mdspan::__src_indices'

  template <typename _Extents, typename... _Slicers> consteval auto __subextents() {
    using _IndexType = _Extents::index_type;
    static constexpr size_t __subrank = ((!convertible_to<_Slicers, _IndexType>)+... + 0);
    static constexpr auto __map_rank = __mdspan::__generate_map_ranks<_IndexType, _Slicers...>();
    __mdspan::__maybe_empty_array<size_t, __subrank> __result;
    if constexpr (__subrank != 0) {
      __mdspan::__for_each_index_pack_and_types<_Extents::rank(), _Slicers...>([&]<size_t _Kp, typename _Slicer> {
        if constexpr (__map_rank[_Kp] != dynamic_extent) {
          if constexpr (is_convertible_v<_Slicer, full_extent_t>)
            __result[__map_rank[_Kp]] = _Extents::static_extent(_Kp);
          else if constexpr (requires {
                               // allows short-circuit evaluation
                               requires __mdspan::__index_pair_like<_Slicer, _IndexType>;
                               requires __mdspan::__integral_constant_like<tuple_element_t<0, _Slicer>>;
                               requires __mdspan::__integral_constant_like<tuple_element_t<1, _Slicer>>;
                             })
            __result[__map_rank[_Kp]] =
                __mdspan::__de_ice(tuple_element_t<1, _Slicer>()) - __mdspan::__de_ice(tuple_element_t<0, _Slicer>());
          else if constexpr (requires {
                               requires __mdspan::__is_strided_slice<_Slicer>;
                               requires __mdspan::__integral_constant_like<typename _Slicer::extent_type>;
                               requires typename _Slicer::extent_type() == 0;
                             })
            __result[__map_rank[_Kp]] = 0;
          else if constexpr (requires {
                               requires __mdspan::__is_strided_slice<_Slicer>;
                               requires __mdspan::__integral_constant_like<typename _Slicer::extent_type>;
                               requires __mdspan::__integral_constant_like<typename _Slicer::stride_type>;
                             })
            __result[__map_rank[_Kp]] =
                1 + (__mdspan::__de_ice(_Slicer::extent_type()) - 1) / __mdspan::__de_ice(_Slicer::stride_type());
          else
            __result[__map_rank[_Kp]] = dynamic_extent;
        }
      });
    }
    return __result;
  }

  template <typename _Extents, typename... _Slicers> consteval auto __submdspan_extents_result_impl() {
    static constexpr auto __subextents_result = __mdspan::__subextents<_Extents, _Slicers...>();
    return __mdspan::__apply_index_pack<__subextents_result.size()>(
        []<size_t... _Idx> { return extents<typename _Extents::index_type, __subextents_result[_Idx]...>{}; });
  }
  } // namespace __mdspan

  template <typename _Extents, typename... _Slicers>
  requires __mdspan::__is_extents<_Extents> &&(_Extents::rank() == sizeof...(_Slicers)) &&
      (__mdspan::__slice_specifier<_Slicers, typename _Extents::index_type> && ...) constexpr auto submdspan_extents(
          const _Extents &__src, _Slicers... __slices) {
    // TODO: add preconditions

    // preconditions:
    // for each rank index k of __src.extents(), all of the following are true:
    // * if _Slicers ... [k] is a specialization of strided_slice:
    //   * __slices ... [k].extent == 0
    //   * __slices ... [k].stride > 0
    // * 0 <= __first_of_extent_slice<_IndexType, k>(__slices...)
    //     <= __last_of_extent_slice<k>(__src, __slices...)
    //     <= __src.extent(k)

    using _Result = decltype(__mdspan::__submdspan_extents_result_impl<_Extents, _Slicers...>());
    if constexpr (_Result::rank() == 0)
      return _Result{};
    else {
      using _IndexType = _Extents::index_type;
      static constexpr auto __map_rank = __mdspan::__generate_map_ranks<_IndexType, _Slicers...>();
      array<_IndexType, _Result::rank()> __result_args;
      __mdspan::__for_each_index_pack_and_args<_Extents::rank()>(
          [&]<size_t _Kp, typename _Slicer>(type_identity_t<_Slicer> __slice) {
            if constexpr (__map_rank[_Kp] != dynamic_extent) {
              if constexpr (__mdspan::__is_strided_slice<_Slicer>)
                __result_args[__map_rank[_Kp]] = __slice.extent == 0 ? 0
                                                                     : 1 + (__mdspan::__de_ice(__slice.extent) - 1) /
                                                                               __mdspan::__de_ice(__slice.stride);
              else
                __result_args[__map_rank[_Kp]] = __mdspan::__last_of_extent_slice<_Kp>(__src, __slices...) -
                                                 __mdspan::__first_of_extent_slice<_IndexType, _Kp>(__slices...);
            }
          },
          __slices...);
      return _Result{__result_args};
    }
  }

  namespace __mdspan {
  template <typename _Extents, typename _Subextents, typename _LayoutMapping, typename... _Slicers>
  constexpr auto __submdspan_strides(const _LayoutMapping &__mapping, _Slicers... __slices) {
    using _IndexType = _Extents::index_type;
    static constexpr auto __map_rank = __mdspan::__generate_map_ranks<_IndexType, _Slicers...>();
    array<_IndexType, _Subextents::rank()> __result;
    if constexpr (_Subextents::rank() != 0) {
      __mdspan::__for_each_index_pack_and_args<_Extents::rank()>(
          [&]<size_t _Kp, typename _Slicer>(type_identity_t<_Slicer> __slice) {
            if constexpr (__map_rank[_Kp] != dynamic_extent) {
              if constexpr (__mdspan::__is_strided_slice<_Slicer>)
                __result[__map_rank[_Kp]] =
                    __mapping.stride(_Kp) * (__slice.stride < __slice.extent ? __mdspan::__de_ice(__slice.stride) : 1);
              else
                __result[__map_rank[_Kp]] = __mapping.stride(_Kp);
            }
          },
          __slices...);
    }
    return __result;
  }

  template <typename _IndexType, typename _Extents, typename _Subextents, typename... _Slicers>
  consteval bool __layout_left_submdspan_mapping_conds() {
    if constexpr (_Subextents::rank() == 0 || _Subextents::rank() - 1 >= sizeof...(_Slicers))
      return false;
    else {
      using _LastSlicer = _Nth_type_t<_Subextents::rank() - 1, _Slicers...>;
      return __mdspan::__apply_index_pack<_Subextents::rank() - 1>([]<size_t... _Ks> {
               return (is_convertible_v<_Nth_type_t<_Ks, _Slicers...>, full_extent_t> && ...);
             }) &&
             (__mdspan::__index_pair_like<_LastSlicer, _IndexType> || is_convertible_v<_LastSlicer, full_extent_t>);
    }
  }

  template <typename _IndexType, typename _Extents, typename _Subextents, typename... _Slicers>
  consteval bool __layout_right_submdspan_mapping_conds() {
    if constexpr (_Extents::rank() < _Subextents::rank() ||
                  _Extents::rank() - _Subextents::rank() >= sizeof...(_Slicers))
      return false;
    else {
      using _FirstSlicer = _Nth_type_t<_Extents::rank() - _Subextents::rank(), _Slicers...>;
      return __mdspan::__apply_index_pack<_Subextents::rank() - 1>([]<size_t... _Ks> {
               constexpr auto __apply_offset = [](size_t _Kp) -> size_t {
                 return _Kp + _Extents::rank() - _Subextents::rank() + 1;
               };
               return (is_convertible_v<_Nth_type_t<__apply_offset(_Ks), _Slicers...>, full_extent_t> && ...);
             }) &&
             (__mdspan::__index_pair_like<_FirstSlicer, _IndexType> || is_convertible_v<_FirstSlicer, full_extent_t>);
    }
  }

  // TODO: add constraints and preconditions
  template <typename _Extents, typename _LayoutMapping, typename... _Slicers>
  constexpr auto __submdspan_mapping_impl(const _LayoutMapping &__mapping, _Slicers... __slices) {
    using _IndexType = _Extents::index_type;
    const auto __subexts = std::submdspan_extents(__mapping.extents(), __slices...);
    using _Subextents = remove_cv_t<decltype(__subexts)>;
    const auto __substrides = __mdspan::__submdspan_strides<_Extents, _Subextents>(__mapping, __slices...);
    return __mdspan::__apply_index_pack<_LayoutMapping::extents_type::rank()>([&]<size_t... _Idx> {
      const size_t __offset = __mapping(__mdspan::__first_of_extent_slice<_IndexType, _Idx>(__slices...)...);
      if constexpr (_Extents::rank() == 0)
        return submdspan_mapping_result{__mapping, 0};
      else if constexpr (is_same_v<layout_left, typename _LayoutMapping::layout_type> &&
                         __mdspan::__layout_left_submdspan_mapping_conds<_IndexType, _Extents, _Subextents,
                                                                         _Slicers...>())
        return submdspan_mapping_result{layout_left::mapping(__subexts), __offset};
      else if constexpr (is_same_v<layout_right, typename _LayoutMapping::layout_type> &&
                         __mdspan::__layout_right_submdspan_mapping_conds<_IndexType, _Extents, _Subextents,
                                                                          _Slicers...>())
        return submdspan_mapping_result{layout_right::mapping(__subexts), __offset};
      else
        return submdspan_mapping_result{layout_stride::mapping(__subexts, __substrides), __offset};
    });
  }
  } // namespace __mdspan

  // TODO: add constraints and preconditions
  template <typename _Tp, typename _Extents, typename _Layout, typename _Accessor, typename... _Slicers>
  constexpr auto submdspan(const mdspan<_Tp, _Extents, _Layout, _Accessor> &__src, _Slicers... __slices) {
    auto __sub_map_offset = submdspan_mapping(__src.mapping(), __slices...);
    return mdspan(__src.accessor().offset(__src.data_handle(), __sub_map_offset.offset), __sub_map_offset.mapping,
                  typename _Accessor::offset_policy(__src.accessor()));
  }

#endif // C++26

  _GLIBCXX_END_NAMESPACE_VERSION
} // namespace )

#endif // __cpp_lib_mdspan
#endif // _GLIBCXX_MDSPAN
