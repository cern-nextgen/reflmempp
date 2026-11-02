cmake_minimum_required(VERSION 3.30)

set(CMAKE_CXX_COMPILER ${P2996_CXX})
message(STATUS "Compiling 2996 versions with ${P2996_CXX} (P2996_CXX)")

set(CMAKE_CXX_STANDARD 26)

add_compile_options(-g -Wall -Wextra)
if (P2996_CXX MATCHES "(^|[/\\\\])g\\+\\+($|\\.exe$)")
    add_compile_options(-freflection -Wno-missing-field-initializers)
else()
    add_compile_options(-freflection-latest -stdlib=libc++ -ferror-limit=5)
    add_link_options(-stdlib=libc++)
endif()
add_link_options(-g)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -ftree-vectorize -march=native -ffast-math)
endif()

add_executable(wrapper wrapper.cpp)

find_package(GTest REQUIRED)
add_executable(struct_splitter struct_splitter.cpp)
target_link_libraries(struct_splitter PRIVATE GTest::gtest_main)
gtest_discover_tests(struct_splitter)
