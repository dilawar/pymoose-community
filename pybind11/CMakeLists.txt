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
    # mimic --whole-archive in MSVC :-/ (requires 2015v2 at least)
    # set_target_properties(_moose PROPERTIES  LINK_FLAGS "/WHOLEARCHIVE")
    target_link_libraries(_moose PRIVATE ${MOOSE_LIBRARIES})
    set(MSVC_WHOLE_ARCHIVE "")
    foreach(_lib ${MOOSE_LIBRARIES})
        message(STATUS " whole archive ${_lib}")
        set(MSVC_WHOLE_ARCHIVE "${MSVC_WHOLE_ARCHIVE} /WHOLEARCHIVE:${_lib}")
    endforeach()
    set_target_properties(_moose PROPERTIES LINK_FLAGS "${MSVC_WHOLE_ARCHIVE}")
endif()

# Move _moose.so to python/moose directory.
# FIXME: LIBRARY_OUTPUT_DIRECTORY does not work with some new cmake
# version. Probably a bug in CMake. Let's just do it explicitly.
add_custom_command(TARGET _moose POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:_moose> ${CMAKE_SOURCE_DIR}/python/moose
    COMMENT "Copying $<TARGET_NAME:_moose> to python src folder"
    )

#
# COPY Python module to binary directory.
# Copy python tree to BUILD directory. User can set PYTHONPATH to
# ${CMAKE_BINARY_DIR}/python and also to ${CMAKE_SOURE_DIR}/python
# Usually its a good practice to keep final module in binary directory.
set(PYMOOSE_BUILD_DIR ${CMAKE_BINARY_DIR}/python)
file(GLOB_RECURSE PY_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../python/*.py)
add_custom_target(_copy_python_source ALL
    DEPENDS _moose  ${PY_SOURCE_FILES}
    # Copy whole Python's source tree to build directory.
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/python/
        ${PYMOOSE_BUILD_DIR}
    COMMENT "pybind11: Copying Python src ${CMAKE_SOURCE_DIR}/python → ${PYMOOSE_BUILD_DIR}"
    VERBATIM)

#
# Create a wheel 
# copy setup.py.in as setup.py file in cwd.
#
configure_file(${CMAKE_SOURCE_DIR}/cmake/setup.py.in ${CMAKE_BINARY_DIR}/setup.py)

add_custom_target(wheel ALL 
    DEPENDS _moose
    # COMMAND ${PYTHON_EXECUTABLE} -m pip install wheel --user
    COMMAND ${PYTHON_EXECUTABLE} setup.py bdist_wheel -d ${CMAKE_BINARY_DIR} 
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "python's wheel is saved to ${CMAKE_BINARY_DIR}"
    VERBATIM)

#
# Install the wheel in a local directory. This directory can be moved by cmake
# using install command to ${CMAKE_INSTALL_PREFIX}. I can't can think of a
# better way to make sure `make install` installs the python package to
# ${CMAKE_INSTALL_PREFIX}

set(PYMOOSE_TEMP_INSTALL_DIR ${CMAKE_BINARY_DIR}/__pymoose_temp_install)
file(MAKE_DIRECTORY ${PYMOOSE_TEMP_INSTALL_DIR})
add_custom_command(TARGET _moose POST_BUILD
    COMMAND ${PYTHON_EXECUTABLE} -m pip install pymoose-*.whl
        --prefix ${PYMOOSE_TEMP_INSTALL_DIR}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Unarchiving ${PYMOOSE_WHEEL_FILE} -> ${PYMOOSE_TEMP_INSTALL_DIR}")

#
# install wheel.
#
install(DIRECTORY ${PYMOOSE_TEMP_INSTALL_DIR}/ DESTINATION ${CMAKE_INSTALL_PREFIX})
