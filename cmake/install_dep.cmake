# install dependencies using conan
find_package(Python3 COMPONENTS Interpreter REQUIRED)

execute_process(COMMAND ${Python3_EXECUTABLE} -m pip install conan)
set(CONAN_CMD ${Python3_EXECUTABLE} -m conans.conan)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/conan.cmake)

conan_cmake_run(REQUIRES 
        boost/1.75.0
        pybind11/2.6.1
        eigen/3.3.9
        fmt/7.1.3
    OPTIONS
        boost:header_only=True
    BASIC_SETUP CMAKE_TARGETS
    BUILD missing
    BUILD_TYPE "${CMAKE_BUILD_TYPE}")
