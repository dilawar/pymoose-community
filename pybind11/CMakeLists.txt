# Add pybind11 module.

pybind11_add_module(_moose 
    pymoose.cpp 
    helper.cpp 
    Finfo.cpp
    MooseVec.cpp 
    PyRun.cpp)

# Use in version_info dict.
target_compile_definitions(_moose PRIVATE USE_FMT_LIB=ON)

set_target_properties(_moose PROPERTIES PREFIX "")
target_link_libraries(_moose PRIVATE CONAN_PKG::pybind11 CONAN_PKG::fmt)

if(APPLE)
    set(CMAKE_MODULE_LINKER_FLAGS "-undefined dynamic_lookup")
    message(STATUS "ADDING some linker flags ${CMAKE_EXE_LINKER_FLAGS}")
    set_target_properties(_moose PROPERTIES MACOSX_RPATH OFF)
endif(APPLE)

if(APPLE)
    # OSX
    target_link_libraries(_moose PRIVATE "-Wl,-all_load" ${MOOSE_LIBRARIES})
    # target_link_libraries(_moose PRIVATE ${SYSTEM_SHARED_LIBS})
elseif(UNIX)
    # Linux
    target_link_libraries(_moose PRIVATE
        "-Wl,--whole-archive" 
        ${MOOSE_LIBRARIES} 
        "-Wl,--no-whole-archive"
        ${SYSTEM_SHARED_LIBS})
elseif(MSVC)
    target_link_libraries(_moose PRIVATE ${MOOSE_LIBRARIES})
    target_link_libraries(_moose PRIVATE ${SYSTEM_SHARED_LIBS})
    # mimic --whole-archive in MSVC :-/ (requires 2015v2 at least)
    foreach(_lib in ${MOOSE_LIBRARIES})
        set_target_properties(_moose PROPERTIES  LINK_FLAGS "/WHOLEARCHIVE:${_lib}")
    endforeach()
endif()

# NOTE: Because LIBRARY_OUTPUT_DIRECTORY does not work with some new cmake
# version.
add_custom_command(TARGET _moose POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:_moose> ${CMAKE_SOURCE_DIR}/python/moose
    COMMENT "Copying $<TARGET_NAME:_moose> to python src folder"
    )

# copy setup.py.in as setup.py file in cwd.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/setup.py.in
    ${CMAKE_BINARY_DIR}/setup.py)


# Copy python tree to BUILD directory. User can set PYTHONPATH to
# ${CMAKE_BINARY_DIR}/python. 
set(PYMOOSE_BUILD_DIR ${CMAKE_BINARY_DIR}/python)
add_custom_target(copy_python_tree ALL
    # Copy whole Python's source tree to build directory.
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/python/
        ${PYMOOSE_BUILD_DIR}
    COMMENT "pybind11: Copying Python src ${CMAKE_SOURCE_DIR}/python → ${PYMOOSE_BUILD_DIR}"
    DEPENDS _moose 
    VERBATIM)

 
# Create a binary distribution inside a directory. Installation will copy that
# directory.
set(_platform "CMAKE")
if(WIN32)
    set(BDIST_EXT "zip")
elseif(UNIX)
    set(BDIST_EXT "tar.gz")
endif()

set(PYMOOSE_BDIST_FILE ${CMAKE_BINARY_DIR}/pymoose-${VERSION_MOOSE}.${_platform}.${BDIST_EXT})
set(PYMOOSE_INSTALL_DIR ${CMAKE_BINARY_DIR}/_pymoose_temp_install)
file(MAKE_DIRECTORY ${PYMOOSE_INSTALL_DIR})

add_custom_target(wheel ALL 
    DEPENDS _moose
    COMMAND ${PYTHON_EXECUTABLE} -m pip install wheel --user
    COMMAND ${PYTHON_EXECUTABLE} setup.py bdist_wheel --skip-build -d ${CMAKE_BINARY_DIR} 
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "python's wheel is saved to ${CMAKE_BINARY_DIR}"
    VERBATIM)

install(DIRECTORY ${PYMOOSE_INSTALL_DIR}/usr/local/
    DESTINATION ${CMAKE_INSTALL_PREFIX}
    CONFIGURATIONS Debug Release)
