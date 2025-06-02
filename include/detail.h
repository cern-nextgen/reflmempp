#ifndef DETAIL_H
#define DETAIL_H

#include "utils.h"
#include <experimental/meta>
#include <iostream>
#include <span>
#include <type_traits>

using namespace std::literals::string_view_literals;

namespace rmpp {
namespace detail {
/// -------------------------------------------------------------------------------------
/// Check if a given reflection is in a vector of reflections.
/// -------
/// Params:
/// -------
/// vec: vector of reflections
/// refl: reflection to check
/// -------
/// Return:
/// -------
/// true if the reflection is in the vector, false otherwise
consteval bool contains_refl(std::vector<std::meta::info> vec, std::meta::info refl) {
  for (auto r : vec) {
    if (r == refl) return true;
  }
  return false;
}

/// -------------------------------------------------------------------------------------
/// Get declarations of value type versions of each data member in S
consteval std::meta::info gen_value_type(std::meta::info S) {
  // List of tokens for SoA member declarations.
  std::vector<std::meta::info> decl_tokens;

  // List of already declared structs in the current scope.
  std::vector<std::meta::info> visited_structs;

  for (auto member : nsdms(S)) {
    auto type = type_remove_cvref(type_of(member));
    auto name = identifier_of(member);

    if (type_is_container(type)) {
      auto value_type = get_scalar_type(type);
      decl_tokens.push_back(^^{ std::vector<typename[:\(value_type):]> \id(name); });
    } else if (type_is_struct(type)) {
      // Get tokens for declaring a version of the nested struct
      // with span members. Skip the declaration if the struct
      // has already been declared in the current scope.
      if (!contains_refl(visited_structs, type)) {
        visited_structs.push_back(type);

        // Build declaration of nested struct.
        auto subdecl_tokens = gen_value_type(type);
        decl_tokens.push_back(^^{ struct \id(identifier_of(type), "SoA"sv){
                                \tokens(subdecl_tokens)}; });
      }

      decl_tokens.push_back(^^{ \id(identifier_of(type), "SoA"sv) \id(name); });

    } else { // Basic data member.
      decl_tokens.push_back(^^{ typename[:\(type):] \id(name); });
    }
  }

  // Concatenate the declarations into a single token string,
  // separated by semicolons.
  std::meta::info decl_concat = ^^{};
  for (auto tks : decl_tokens) {
    decl_concat = ^^{ \tokens(decl_concat) \tokens(tks) };
  }

  return decl_concat;
}

/// -------------------------------------------------------------------------------------
/// Injects declarations for a Structure of Arrays (SoA) version of the AoS
/// members. If a member is a struct, it recursively generates a nested struct
/// containing the span versions of the submembers. The nested struct has the
/// same name as the original struct with "SoA" appended.
/// -------
/// Params:
/// -------
/// S: Struct used for AoS
/// inject: If true, inject the declarations into the current scope
/// -------
/// Return:
/// -------
/// Tokens for declarations of SoA members in the current scope.
consteval std::meta::info gen_soa_members(std::meta::info S, bool inject) {
  // List of tokens for SoA member declarations.
  std::vector<std::meta::info> decl_tokens;

  // List of already declared structs in the current scope.
  std::vector<std::meta::info> visited_structs;

  for (auto member : nsdms(S)) {
    auto type = type_remove_cvref(type_of(member));
    auto name = identifier_of(member);

    if (type_is_container(type)) {
      auto value_type = get_scalar_type(type);
      decl_tokens.push_back(^^{ std::span<typename[:\(value_type):]> \id(name); });
      decl_tokens.push_back(^^{ std::vector<size_t> \id(name, "_offsets"sv); });
    } else if (type_is_struct(type)) {
      // Get tokens for declaring a version of the nested struct
      // with span members. Skip the declaration if the struct
      // has already been declared in the current scope.
      if (!contains_refl(visited_structs, type)) {
        visited_structs.push_back(type);

        // Build declaration of nested struct.
        auto subdecl_tokens = gen_soa_members(type, false);
        decl_tokens.push_back(^^{ struct \id(identifier_of(type), "SoA"sv){
                                \tokens(subdecl_tokens)}; });
      }

      decl_tokens.push_back(^^{ \id(identifier_of(type), "SoA"sv) \id(name); });

    } else { // Basic data member.
      decl_tokens.push_back(^^{ std::span<typename[:\(type):]> \id(name); });
    }
  }

  // Concatenate the declarations into a single token string,
  // separated by semicolons.
  std::meta::info decl_concat = ^^{};
  for (auto tks : decl_tokens) {
    decl_concat = ^^{ \tokens(decl_concat) \tokens(tks) };
  }

  // Inject the declarations.
  // Only inject if this is the top-level struct, otherwise we return
  // the tokens, so they can be wrapped inside the declaration of the
  // parent struct.
  if (inject) { __report_and_inject(decl_concat); }

  return decl_concat;
}

/// -------------------------------------------------------------------------------------
/// Generate initialization of a given SoA member.
/// -------
/// Params:
/// -------
/// member: reflection of the corresponding AoS member
/// id_tokens: tokens for the identifier of the current member
/// offset_tokens: tokens for the offset into the storage for the current span
/// -------
/// Return:
/// -------
/// current total offset into the storage
consteval std::meta::info gen_soa_init(std::meta::info member, std::meta::info id_tokens,
                                       std::meta::info offset_tokens) {
  auto soa_member = identifier_of(member);
  auto type = type_remove_cvref(type_of(member));

  if (type_is_container) {
    auto value_type = get_scalar_type(type);

    // Inject span initialization for the current vector member.
    __report_and_inject(^^{
      \tokens(id_tokens) =
          std::span(reinterpret_cast<typename[: \(value_type):] *>(storage.data() + \tokens(offset_tokens)),
                    m_capacity * max_inner_array_size);
    });

    // +1 to be able to calculate the size of the last data element based on the difference
    // in the offset of the next element and the current element.
    __report_and_inject(^^{ \id(soa_member, "_offsets"sv).resize(m_capacity + 1); });

    //   Tokens for computing the offset into the storage for the next member.
    offset_tokens = ^^{ \tokens(offset_tokens) + \tokens(id_tokens).size_bytes() };
  } else if (type_is_struct(type)) {
    // Recursively initialize nested struct members. Add the identifier of
    // the current member to the id_tokens to access the members of the
    // nested struct.
    for (auto submember : nsdms(type)) {
      auto offset = gen_soa_init(submember, ^^{
                \tokens(id_tokens).\id(identifier_of(submember))}, offset_tokens);
      offset_tokens = ^^{ \tokens(offset_tokens) + \tokens(offset) };
    }
  } else { // Scalar members
    // Inject span initialization for the current member.
    __report_and_inject(^^{
      \tokens(id_tokens) =
          std::span(reinterpret_cast<typename[: \(type):] *>(storage.data() + \tokens(offset_tokens)), m_capacity);
    });

    // Tokens for computing the offset into the storage for the next member.
    offset_tokens = ^^{ \tokens(offset_tokens) + \tokens(id_tokens).size_bytes() };
  }

  return offset_tokens;
}

consteval void gen_push_back(std::meta::info member) {
  auto name = identifier_of(member);
  auto type = type_of(member);

  if (type_is_container(type)) {
    auto value_type = get_scalar_type(type);

    __report_and_inject(^^{
      for (size_t i = 0; i < obj.\id(name).size(); i++) {
        new (&\id(name)[\id(name, "_offsets"sv)[m_size] + i]) typename[: \(value_type):](obj.\id(name)[i]);
      }
    });

    __report_and_inject(
        ^^{ \id(name, "_offsets"sv)[m_size + 1] = \id(name, "_offsets"sv)[m_size] + obj.\id(name).size(); });
  } else if (type_is_struct(type)) {

  } else {
    __report_and_inject(^^{ new (&\id(name)[m_size]) typename[: \(type):](obj.\id(name)); });
  }
}

namespace views {

/// -------------------------------------------------------------------------------------
/// Get the tokens needed to initialize the view of a given AoS member.
/// -------
/// Params:
/// -------
/// member: member to generate view for
/// idx: idx of the current element in the AoS
/// id_tokens: tokens for the identifier of the current (nested) member
///            tokens for initializing the view of the current member
consteval std::meta::info assign_aos_view_member(std::meta::info member, std::meta::info idx,
                                                 std::meta::info id_tokens) {
  auto name = identifier_of(member);
  auto type = type_of(member);

  if (type_is_container(type)) {
    auto offset = ^^{ \id(name, "_offsets"sv)[\id(identifier_of(idx))] };
    auto size = ^^{ \id(name, "_offsets"sv)[\id(identifier_of(idx)) + 1] - \tokens(offset) };
    return ^^{ \tokens(id_tokens).subspan(\tokens(offset), \tokens(size)) };
  } else if (type_is_struct(type)) {
    // Recursively initialize nested struct members. Add the identifier of
    // the current member to the id_tokens to access the members of the
    // nested struct.
    std::meta::list_builder substruct_assign{};
    for (auto submember : nsdms(type)) {
      auto submember_id = ^^{ \tokens(id_tokens).\id(identifier_of(submember)) };
      substruct_assign += assign_aos_view_member(submember, idx, submember_id);
    }

    // Nested struct members. e.g., .vecs = { .a = a[idx], .b = b[idx] }
    return ^^{
      { \tokens(substruct_assign) }
    };
  }

  // Scalar members. e.g., .x = x[idx]
  return ^^{ \tokens(id_tokens)[\id(identifier_of(idx))] };
}

} // namespace views
} // namespace detail
} // namespace rmpp

#endif