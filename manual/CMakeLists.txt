set(NO_REFL_CXX "/usr/bin/g++-14")
set(CMAKE_CXX_COMPILER ${NO_REFL_CXX})
message(STATUS "Compiling manual versions with ${NO_REFL_CXX} (NO_REFL_CXX)")
set(CMAKE_CXX_STANDARD 26)

set(MANUAL_SOURCES
    aos2soa
    aos2soa_contiguous
    aos2soa_containers
)

foreach(src ${MANUAL_SOURCES})
    add_executable(${src}_norefl ${src}.cpp)
endforeach()
