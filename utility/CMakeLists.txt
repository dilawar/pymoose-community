cmake_minimum_required(VERSION 3.12)

add_library(utility
    strutil.cpp
    types.cpp
    setupenv.cpp
    numutil.cpp
    Annotator.cpp
    Vec.cpp
    cnpy.cpp
    utility.cpp)

add_executable(test_cnpy test_cnpy.cpp)
target_link_libraries(test_cnpy utility)

add_executable(test_util test_util.cpp)
target_link_libraries(test_util utility)

enable_testing()
add_test(NAME cpp_test_cnpy COMMAND $<TARGET_FILE:test_cnpy>)
add_test(NAME cpp_test_util  COMMAND $<TARGET_FILE:test_util>)
