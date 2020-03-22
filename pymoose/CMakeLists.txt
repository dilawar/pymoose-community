cmake_minimum_required(VERSION 2.8)

include(${CMAKE_CURRENT_SOURCE_DIR}/../CheckCXXCompiler.cmake)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake_modules/")

pybind11_add_module(_moose pymoose.cpp)
set_target_properties(_moose
    PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/python/moose)

target_link_libraries(_moose PRIVATE moose)

enable_testing()
add_test(NAME sanity 
    COMMAND ${PYTHON_EXECUTABLE} -c ${CMAKE_CURRENT_SOURCE_DIR}/test_pybind_moose.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
set_tests_properties(sanity
    PROPERTIES ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/pymoose"
    )

# Copy python tree to BUILD directory. User can set PYTHONPATH to
# ${CMAKE_BINARY_DIR}/python.
add_custom_target(copy_python_tree ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/python ${CMAKE_BINARY_DIR}/python
    COMMENT "Copying python source tree: ${CMAKE_SOURCE_DIR}/python -> ${CMAKE_BINARY_DIR}/python"
    DEPENDS _moose 
    VERBATIM)
