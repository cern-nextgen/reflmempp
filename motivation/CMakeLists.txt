cmake_minimum_required(VERSION 3.30)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_PREFIX_PATH "~/apps/benchmark/")
list(APPEND CMAKE_MODULE_PATH "~/apps/benchmark/cmake/Modules")

set(INTEL_FOLDER /home/jollychen/apps/intel/oneapi/compiler/2025.0)
set(GCC_FOLDER /usr/bin)
set(CLANG_FOLDER /usr/bin)

if(intel)
    set(CMAKE_CXX_COMPILER ${INTEL_FOLDER}/bin/icpx)
    set(CMAKE_C_COMPILER ${INTEL_FOLDER}/bin/icx)
elseif(gcc)
    set(CMAKE_CXX_COMPILER ${GCC_FOLDER}/g++-14)
    set(CMAKE_C_COMPILER ${GCC_FOLDER}/gcc-14)
elseif(clang)
    set(CMAKE_CXX_COMPILER ${CLANG_FOLDER}/clang++-18)
    set(CMAKE_C_COMPILER ${CLANG_FOLDER}/clang-18)
    set(LLVMAR_EXECUTABLE ${CLANG_FOLDER}/llvm-ar-18)
    set(LLVMNM_EXECUTABLE ${CLANG_FOLDER}/llvm-nm-18)
endif()

add_compile_options(-O3 -Wall -Wextra -Wpedantic)
if (novec) 
    message(STATUS "Disabling vectorization")
    if(intel)
        add_compile_options(-no-vec)
    elseif(gcc)
        add_compile_options(-fno-tree-vectorize)
    elseif(clang)
        add_compile_options(-fno-vectorize -fno-slp-vectorize)
    endif()
else()
    message(STATUS "Enabling vectorization")
    add_compile_options(-march=skylake -ftree-vectorize)
endif()

project(
    motivation
    LANGUAGES CXX
)

find_package(benchmark REQUIRED)

add_executable(aos2soa aos2soa.cpp)

target_link_libraries(aos2soa PRIVATE benchmark::benchmark benchmark::benchmark_main)
if(intel)
    # target_compile_options(aos2soa PRIVATE  -qopt-report3 -qopt-report-file=opt-aos2soa.txt )
elseif(clang)
    target_compile_options(aos2soa PRIVATE -fexperimental-library)
endif()


