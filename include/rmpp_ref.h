#ifndef REFLMEMPP_H
#define REFLMEMPP_H

#include "utils.h"
#include <experimental/meta>
#include <iostream>
#include <memory>
#include <memory_resource>
#include <span>
#include <type_traits>

namespace rmpp {
using namespace std::literals::string_view_literals;

constexpr auto member_prefix = ""sv;
constexpr auto metadata_suffix = "_md"sv;
constexpr auto extents_suffix = "_extents"sv;

namespace detail {

/// @brief Check if a given reflection is in a vector of reflections.
/// @param vec vectro of reflections
/// @param refl reflection to check
/// @return true if the reflection is in the vector, false otherwise
consteval bool contains_refl(std::vector<std::meta::info> vec, std::meta::info refl) {
  for (auto r : vec) {
    if (r == refl) return true;
  }
  return false;
}

/// @brief Injects declarations for a Structure of Arrays (SoA) version of a AoS
/// member. If the member is a struct, it recursively generates a nested struct
/// containing the span versions of the submembers. The nested struct has the
/// same name as the original struct with "SoA" appended.
/// @param member AoS member to generate SoA version for
/// @param declare if true, declare a SoA version of the nested struct
/// @param inject if true, inject the declaration into the current scope
/// @return
consteval std::meta::info gen_soa_decl(std::meta::info member, bool declare, bool inject) {
  auto type = type_remove_cvref(type_of(member));
  auto name = identifier_of(member);

  std::meta::info decl_tokens = ^{};
  if (type_is_struct(type)) {
    // Declare a version of the nested struct with span members. Skip
    // the declaration if the struct has already been declared in the
    // current scope.
    if (declare) {
      // List of already declared structs in the current scope.
      std::vector<std::meta::info> declared;

      // Get span declarations for each member in the nested struct.
      std::vector<std::meta::info> subdecl_tokens;
      for (auto submember : nonstatic_data_members_of(type)) {
        if (contains_refl(declared, type_of(submember))) {
          subdecl_tokens.push_back(gen_soa_decl(submember, false, false));
        } else {
          declared.push_back(type_of(submember));
          subdecl_tokens.push_back(gen_soa_decl(submember, true, false));
        }
      }

      // Concatenate the declarations into a single token string,
      // separated by semicolons.
      std::meta::info subdecl_concat = ^{};
      for (auto tks : subdecl_tokens) {
        subdecl_concat = ^{ \tokens(subdecl_concat) \tokens(tks) };
      }

      // Build declaration of nested struct.
      decl_tokens = ^{ struct \id(identifier_of(type), "SoA"sv){
                    \tokens(subdecl_concat)}; };
    }

    // Inject the declaration of the nested struct and an instantiation.
    // Only inject if this is the top-level struct, otherwise we return
    // the tokens, so they can be wrapped inside the declaration of the
    // parent struct
    if (inject) {
      // __report_tokens(decl_tokens);
      queue_injection(decl_tokens);

      // __report_tokens(^{
      //   \id(identifier_of(type), "SoA"sv) \id(name);
      // });
      queue_injection(^{ \id(identifier_of(type), "SoA"sv) \id(name); });
    } else {
      decl_tokens = ^{ \tokens(decl_tokens)
                \id(identifier_of(type), "SoA"sv) \id(name); };
    }
  } else { // Scalar data member.
    decl_tokens = ^{ std::span<typename[:\(type):]> \id(name); };

    if (inject) {
      queue_injection(decl_tokens);
      // __report_tokens(decl_tokens);
    }
  }

  return decl_tokens;
}

/// @brief Generate initalization of a given SoA member.
/// @param member reflection of the corresponding AoS member
/// @param id_tokens tokens for the identifier of the current member
/// @param offset_tokens tokens for the offset into the storage for the current
/// span
/// @return current total offset into the storage
consteval std::meta::info gen_soa_init(std::meta::info member, std::meta::info id_tokens,
                                       std::meta::info offset_tokens) {
  auto id = identifier_of(member);
  auto type = type_remove_cvref(type_of(member));

  if (type_is_struct(type)) {
    // Recursively initialize nested struct members. Add the identifier of
    // the current member to the id_tokens to access the members of the
    // nested struct.
    for (auto submember : nonstatic_data_members_of(type)) {
      auto offset = gen_soa_init(submember, ^{
                \tokens(id_tokens).\id(identifier_of(submember))}, offset_tokens);
      offset_tokens = ^{ \tokens(offset_tokens) + \tokens(offset) };
    }
  } else { // Scalar members
    // Tokens for comuting the offset into the storage for the current
    // member.
    offset_tokens = ^{ \tokens(offset_tokens) + m_size * sizeof(typename[: \(type):]) };

    // Inject span initialization for the current member.
    queue_injection(^{
      \tokens(id_tokens) =
          std::span(reinterpret_cast<typename[: \(type):] *>(storage.data() + \tokens(offset_tokens)), m_size);
    });
  }

  return offset_tokens;
}

namespace views {

/// @brief Get the tokens needed to initialize the view of a given AoS member.
/// @param member member to generate view for
/// @param idx idx of the current element in the AoS
/// @param id_tokens tokens for the identifier of the current (nested) member
/// @return tokens for initializing the view of the current member
consteval std::meta::info assign_aos_view_member(std::meta::info member, std::meta::info idx,
                                                 std::meta::info id_tokens) {
  auto name = identifier_of(member);
  auto type = type_of(member);

  if (type_is_struct(type)) {
    // Recursively initialize nested struct members. Add the identifier of
    // the current member to the id_tokens to access the members of the
    // nested struct.
    std::meta::list_builder substruct_assign{};
    for (auto submember : nonstatic_data_members_of(type)) {
      auto submember_id = ^{ \tokens(id_tokens).\id(identifier_of(submember)) };
      substruct_assign += assign_aos_view_member(submember, idx, submember_id);
    }

    // Nested struct members. e.g., .vecs = { .a = a[idx], .b = b[idx] }
    return ^{
      {
              \tokens(substruct_assign) }
    };
  }

  // Scalar members. e.g., .x = x[idx]
  return ^{ \tokens(id_tokens)[\id(identifier_of(idx))] };
}

} // namespace views
} // namespace detail

template <typename S, size_t Alignment>
class vector {
  static_assert(type_is_struct(^S));

private:
  std::vector<std::byte> storage;
  size_t m_size;

  struct sov_metadata {
    size_t offset, size;
    friend std::ostream &operator<<(std::ostream &os, const sov_metadata &obj) {
      return os << "{" << obj.offset << ", " << obj.size << "}";
    }
  };

  constexpr inline size_t align_size(size_t size, size_t alignment) const {
    return ((size + alignment - 1) / alignment) * alignment;
  }

  /// @brief Compute the size in bytes for a SoA member. If the member is a
  /// struct, the size is computed recursively by summing the sizes of the
  /// scalar submembers.
  /// @tparam Member member to compute the size for
  /// @return size in bytes
  template <std::meta::info Member>
  constexpr size_t compute_size() {
    constexpr auto id = identifier_of(Member);
    constexpr auto type = type_remove_cvref(type_of(Member));

    if constexpr (type_is_struct(type)) {
      size_t struct_size = 0;
      [:expand(nonstatic_data_members_of(type)):] >> [&]<auto submember> { struct_size += compute_size<submember>(); };

      return struct_size;
    }

    return m_size * sizeof(typename[:type_of(Member):]);
  }

  /// @brief Generate a view of a given AoS element.
  /// @tparam ViewType type of the view (e.g., AoS or AoS const)
  /// @param idx index of the element in the AoS
  /// @return view of the element
  template <typename ViewType>
  ViewType generate_view(const size_t idx) const {
    consteval {
      std::meta::list_builder member_data_tokens{};
      for (auto member : nonstatic_data_members_of(^S)) {
        member_data_tokens += detail::views::assign_aos_view_member(member, ^idx, ^{ \id(identifier_of(member))});
      }

      // __report_tokens(member_data_tokens);
      queue_injection(^{ return ViewType{\tokens(member_data_tokens)}; });
    }
  }

public:
  consteval {
    for (auto member : nonstatic_data_members_of(^S)) {
      detail::gen_soa_decl(member, true, true);
    }
  }

  std::vector<size_t> byte_sizes;

  using aos_view = S;
  using aos_cview = const S;

  /// @brief Constructor for the SoA vector.
  /// @param n number of elements in the AoS.
  vector(const size_t n) {
    m_size = n;

    // Compute the size of the storage container.
    size_t total_byte_size = 0;
    [:expand(nonstatic_data_members_of(^S)):] >> [&]<auto member> { total_byte_size += compute_size<member>(); };
    std::cout << "storage of " << total_byte_size << " bytes in total\n\n ";
    storage.resize(total_byte_size);

    // Initialze the spans for each member in the SoA to offsets into
    // the storage container.
    consteval {
      std::meta::info offset_tokens = ^{ 0 };
      for (auto member : nonstatic_data_members_of(^S)) {
        offset_tokens = detail::gen_soa_init(member, ^{ \id(identifier_of(member))}, offset_tokens);
      }

      // __report_tokens(offset_tokens);
    }
  }

  size_t size() const { return m_size; }

  aos_view operator[](const size_t idx) { return generate_view<aos_view>(idx); }

  aos_cview operator[](const size_t idx) const { return generate_view<aos_cview>(idx); }
};
} // namespace rmpp
#endif
