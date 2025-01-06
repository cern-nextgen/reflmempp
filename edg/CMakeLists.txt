set(EDG_CXX "/home/jollychen/apps/edg-compiler.2024.8.15/bin/eccp-shim")
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
endforeach()

