set(CMAKE_CXX_COMPILER ${EDG_CXX})
set(CMAKE_DEPFILE_FLAGS_CXX "")
message(STATUS "Compiling EDG versions with ${EDG_CXX} (EDG_CXX)")

# set(CMAKE_CXX_STANDARD 26)

set(EDG_SOURCES
    aos2soa
)

foreach(src ${EDG_SOURCES})
    add_executable(${src} ${src}.cpp)
    target_compile_options(${src} PUBLIC --driver_debug)
endforeach()

