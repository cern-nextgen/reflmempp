set(CMAKE_CXX_COMPILER ${EDG_CXX})
set(CMAKE_DEPFILE_FLAGS_CXX "")
message(STATUS "Compiling EDG versions with ${EDG_CXX} (EDG_CXX)")

# set(CMAKE_CXX_STANDARD 26)

set(EDG_SOURCES
    aos2soa
)

# Do not add -Wl, rpath flags to the linker command line (edg_prelink does not support them)
set(CMAKE_SKIP_RPATH TRUE)
# Do not add -Wl, --dependency-file flag to the edg_prelink command
set(CMAKE_LINK_DEPENDS_USE_LINKER FALSE)

foreach(src ${EDG_SOURCES})
    add_executable(${src} ${src}.cpp)
    target_compile_options(${src} PUBLIC --driver_debug)
    target_include_directories(${src} PUBLIC include include/kokkos/include/mdspan)
endforeach()

