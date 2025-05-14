#ifndef DETAIL_H
#define DETAIL_H

#include <experimental/meta>
#include <iostream>
#include <span>
#include <type_traits>

using namespace std::literals::string_view_literals;

namespace rmpp {
namespace detail {

/// @brief Check if a given reflection is in a vector of reflections.
/// @param vec vector of reflections
/// @param refl reflection to check
/// @return true if the reflection is in the vector, false otherwise
consteval bool contains_refl(std::vector<std::meta::info> vec, std::meta::info refl) {
  for (auto r : vec) {
    if (r == refl) return true;
  }
  return false;
}

/// @brief Injects declarations for a Structure of Arrays (SoA) version of the AoS
/// members. If a member is a struct, it recursively generates a nested struct
/// containing the span versions of the submembers. The nested struct has the
/// same name as the original struct with "SoA" appended.
/// @param S Struct used for AoS
/// @param inject If true, inject the declarations into the current scope
/// @return Tokens for declarations of SoA members in the current scope.
consteval std::meta::info gen_soa_members(std::meta::info S, bool inject) {
  // List of tokens for SoA member declarations.
  std::vector<std::meta::info> decl_tokens;

  // List of already declared structs in the current scope.
  std::vector<std::meta::info> visited_structs;

  for (auto member : nonstatic_data_members_of(S)) {
    auto type = type_remove_cvref(type_of(member));
    auto name = identifier_of(member);

    if (type_is_struct(type)) {
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
  if (inject) {
    __report_tokens(decl_concat);
    queue_injection(decl_concat);
  }

  return decl_concat;
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
      auto offset = gen_soa_init(submember, ^^{
                \tokens(id_tokens).\id(identifier_of(submember))}, offset_tokens);
      offset_tokens = ^^{ \tokens(offset_tokens) + \tokens(offset) };
    }
  } else { // Scalar members
    // Tokens for computing the offset into the storage for the current
    // member.
    offset_tokens = ^^{ \tokens(offset_tokens) + m_size * sizeof(typename[: \(type):]) };

    // Inject span initialization for the current member.
    queue_injection(^^{
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

  if (type_is_container(type)) {

  } else if (type_is_struct(type)) {
    // Recursively initialize nested struct members. Add the identifier of
    // the current member to the id_tokens to access the members of the
    // nested struct.
    std::meta::list_builder substruct_assign{};
    for (auto submember : nonstatic_data_members_of(type)) {
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
}

#endif