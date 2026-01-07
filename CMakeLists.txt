cmake_minimum_required(VERSION 3.30)

project(
    mds
    LANGUAGES CXX
)

include_directories(include/kokkos/include/mdspan)
include_directories(include/)

add_subdirectory(manual)
add_subdirectory(edg)
add_subdirectory(clang-p2996)
