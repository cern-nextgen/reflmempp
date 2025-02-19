set(CMAKE_CXX_COMPILER ${EDG_CXX})
set(CMAKE_DEPFILE_FLAGS_CXX "")
message(STATUS "Compiling EDG versions with ${EDG_CXX} (EDG_CXX)")

# set(CMAKE_CXX_STANDARD 26)

set(EDG_SOURCES
    aos2soa
    aos2soa_contiguous
    aos2soa_containers
    with_methods
)

foreach(src ${EDG_SOURCES})
    add_executable(${src} ${src}.cpp)
    target_compile_definitions(${src} PRIVATE -D__cpp_lib_reflection)
endforeach()

