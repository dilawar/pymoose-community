cmake_minimum_required(VERSION 2.8)

include(${CMAKE_CURRENT_SOURCE_DIR}/../CheckCXXCompiler.cmake)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake_modules/")

pybind11_add_module(_moose pymoose.cpp helper.cpp)

# set_target_properties(_moose PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/python/moose)
set_target_properties(_moose PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/python/moose
    SUFFIX ".so")

add_dependencies(_moose libmoose)

if(APPLE)
    set(CMAKE_MODULE_LINKER_FLAGS "-undefined dynamic_lookup")
    message(STATUS "ADDING some linker flags ${CMAKE_EXE_LINKER_FLAGS}")
    # cmake --help-policy CMP0042
    set_target_properties( _moose PROPERTIES MACOSX_RPATH OFF)
endif(APPLE)

if(APPLE)
    target_link_libraries(_moose PRIVATE
        "-Wl,-all_load"
        ${MOOSE_LIBRARIES}
        ${STATIC_LIBRARIES})
    target_link_libraries(_moose ${SYSTEM_SHARED_LIBS})
else(APPLE)
    target_link_libraries(_moose PRIVATE
        "-Wl,--whole-archive" 
        ${MOOSE_LIBRARIES} 
        ${STATIC_LIBRARIES} 
        "-Wl,--no-whole-archive"
        ${SYSTEM_SHARED_LIBS})
endif()


enable_testing()
add_test(NAME sanity 
    COMMAND ${PYTHON_EXECUTABLE} -c ${CMAKE_SOURCE_DIR}/tests/pybind11/test_pybind_moose.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
set_tests_properties(sanity
    PROPERTIES ENVIRONMENT "PYTHONPATH=${CMAKE_BINARY_DIR}/python"
    )

# # Copy python tree to BUILD directory. User can set PYTHONPATH to
# # ${CMAKE_BINARY_DIR}/python.
# add_custom_target(copy_python_tree ALL
#     COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/python ${CMAKE_BINARY_DIR}/python
#     COMMENT "Copying python source tree: ${CMAKE_SOURCE_DIR}/python -> ${CMAKE_BINARY_DIR}/python"
#     DEPENDS _moose 
#     VERBATIM)
