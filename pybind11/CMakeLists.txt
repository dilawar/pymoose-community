# Add pybind11 module.
find_package(pybind11 REQUIRED)
find_package(fmt REQUIRED)

pybind11_add_module(_moose
    pymoose.cpp
    helper.cpp
    Finfo.cpp
    MooseVec.cpp
    PyRun.cpp)

# Use in version_info dict.
target_compile_definitions(_moose PRIVATE USE_FMT_LIB=ON)

set_target_properties(_moose PROPERTIES PREFIX "")
target_link_libraries(_moose PRIVATE pybind11::pybind11 fmt::fmt)

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
    COMMENT "Copying _moose module to python src folder"
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
    )

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
# Install the wheel in a local directory.
#
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/install_wheel.py.in
    ${CMAKE_BINARY_DIR}/install_wheel.py)

install(CODE "execute_process(COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_BINARY_DIR}/install_wheel.py)")
