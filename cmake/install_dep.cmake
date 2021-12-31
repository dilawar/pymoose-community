# install dependencies using conan
find_package(Python3 COMPONENTS Interpreter REQUIRED)

#
# Setup conan.
#
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.16.1/conan.cmake"
        "${CMAKE_BINARY_DIR}/conan.cmake"
        EXPECTED_HASH SHA256=396e16d0f5eabdc6a14afddbcfff62a54a7ee75c6da23f32f7a31bc85db23484
        TLS_VERIFY ON)
endif()

find_program(CONAN_COMMAND NAMES conan conan.exe PATHS "C:/Program Files/Conan/conan")
include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(REQUIRES
        boost/1.75.0
        pybind11/2.8.1
        eigen/3.3.9
        fmt/7.1.3
    OPTIONS
        boost:header_only=True
    BASIC_SETUP CMAKE_TARGETS
    BUILD missing
    BUILD_TYPE "${CMAKE_BUILD_TYPE}")
