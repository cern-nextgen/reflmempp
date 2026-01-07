cmake_minimum_required(VERSION 3.30)

set(CMAKE_CXX_COMPILER ${CLANG_2996_CXX})
message(STATUS "Compiling clang-p2996 versions with ${CLANG_2996_CXX} (CLANG_2996_CXX)")

set(CMAKE_CXX_STANDARD 26)

add_compile_options(-g -Wall -Wextra -freflection-latest -stdlib=libc++)
add_link_options(-stdlib=libc++)

add_executable(wrapper wrapper.cpp)
