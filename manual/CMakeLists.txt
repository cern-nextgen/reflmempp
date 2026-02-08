message(STATUS "Compiling manual versions with ${CMAKE_CXX_COMPILER} (CMAKE_CXX_COMPILER)")
set(CMAKE_CXX_STANDARD 26)

set(MANUAL_SOURCES
    edg_aos2soa
)

include_directories(../edg/include)
include_directories(../edg/include/kokkos/include/mdspan)

foreach(src ${MANUAL_SOURCES})
    add_executable(${src}_norefl ${src}.cpp)
endforeach()
