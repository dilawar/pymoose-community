cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

# Project to build MOOSE's python module.
project(PyMOOSE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# cmake related macros.
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# We find python executable here. Though mainly used inside pymoose. 
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
set(CMAKE_MACOSX_RPATH OFF)

if(NOT VERSION_MOOSE)
    string(TIMESTAMP TODAY "%Y%m%d")
    set(VERSION_MOOSE "3.2.2dev${TODAY}")
endif()

add_definitions(-DMOOSE_VERSION="${VERSION_MOOSE}")
message(STATUS "MOOSE Version ${VERSION_MOOSE}")

################################ CMAKE OPTIONS ##################################
option(DEBUG                "Build with debug support" OFF)
option(GPROF                "Build for profiling using gprof" OFF)
option(ENABLE_UNIT_TESTS    "Enable unit tests (DEBUG should also be ON)" OFF)
option(WITH_MPI             "Enable Openmpi support" OFF)
option(WITH_ASAN            "Use AddressSanitizer in DEBUG mode." OFF)
option(WITH_NSDF            "Enable NSDF support. Requires hdf5" OFF )
option(WITH_LEGACY_BINDING  "Use legacy python-bindings" OFF)
option(USE_CONAN            "Use conan to download dependencies" ON)

############################ COMPILER OPTIONS ####################################
if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W1")
endif()

############################ BUILD CONFIGURATION #################################

# include conan support.
if(USE_CONAN)
    include(install_dep)
else(USE_CONAN)
    find_package(Boost 1.53 REQUIRED)
    find_package(Pybind11 REQUIRED)
    find_package(Eigen3 REQUIRED)
endif(USE_CONAN)

# Default definitions.
add_definitions(-DUSE_GENESIS_PARSER)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    message(STATUS "Building for Debug/Unit testing")
    add_definitions(-DDO_UNIT_TESTS)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(WITH_ASAN AND "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    message(STATUS "Compiling with ASAN")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} \
        -fno-omit-frame-pointer -fsanitize=leak -fsanitize=address")
    set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} \
        -fno-omit-frame-pointer -fsanitize=leak -fsanitize=address")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fsanitize=address")
endif()



################################### SETUP BUILD ################################

# This directory have some external dependencies. It also sets some default
# include paths.
add_subdirectory(external)

# Mostly development related scripts. 
add_subdirectory(devel)

# Source code.
add_subdirectory(basecode)
add_subdirectory(msg)
add_subdirectory(shell)
add_subdirectory(randnum)
add_subdirectory(scheduling)
add_subdirectory(biophysics)
add_subdirectory(builtins)
add_subdirectory(utility)
add_subdirectory(mesh)
add_subdirectory(mpi)
add_subdirectory(signeur)
add_subdirectory(ksolve)
add_subdirectory(hsolve)
add_subdirectory(diffusion)
add_subdirectory(device)
add_subdirectory(kinetics)
add_subdirectory(synapse)
add_subdirectory(intfire)

################################### TARGETS ####################################

list(APPEND MOOSE_LIBRARIES
    moose_builtins
    msg
    mesh
    shell
    randnum
    scheduling
    moose_mpi
    biophysics
    utility
    kinetics
    synapse
    intfire
    hsolve
    signeur
    diffusion
    ksolve
    lsoda
    device
    basecode
    )

find_package(Threads REQUIRED)

set(SYSTEM_SHARED_LIBS Threads::Threads)

add_executable(moose.bin basecode/main.cpp)
if(APPLE)
    target_link_libraries(moose.bin PRIVATE "-Wl,-all_load" ${MOOSE_LIBRARIES})
    target_link_libraries(moose.bin PRIVATE ${SYSTEM_SHARED_LIBS})
elseif(UNIX)
    target_link_libraries(moose.bin PRIVATE
        "-Wl,--whole-archive" 
        ${MOOSE_LIBRARIES} 
        "-Wl,--no-whole-archive"
        ${SYSTEM_SHARED_LIBS})
elseif(MSVC)
    # mimic --whole-archive in MSVC :-/ (requires 2015v2 at least)
    # set_target_properties(_moose PROPERTIES  LINK_FLAGS "/WHOLEARCHIVE")
    target_link_libraries(moose.bin PRIVATE ${MOOSE_LIBRARIES} ${SYSTEM_SHARED_LIBS})
    set(MSVC_WHOLE_ARCHIVE "")
    foreach(_lib ${MOOSE_LIBRARIES})
        # message(STATUS " whole archive ${_lib}")
        set(MSVC_WHOLE_ARCHIVE "${MSVC_WHOLE_ARCHIVE} /WHOLEARCHIVE:${_lib}")
    endforeach()
    set_target_properties(moose.bin PROPERTIES LINK_FLAGS "${MSVC_WHOLE_ARCHIVE}")
endif()


################################# PYTHON BINDINGS #############################
add_subdirectory(pybind11)

######################### INSTALL ##############################################

install(TARGETS moose.bin DESTINATION bin CONFIGURATIONS Debug)


############################ CTEST ######################################
enable_testing()
add_subdirectory(tests)

####################### DEVELOPMENT ########################################

# PYLINT target.
set(PYSCRIPTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/python)
file(GLOB_RECURSE PY_SCRIPTS  "python/*.py")
add_custom_target(pylint)
foreach(_py_script ${PY_SCRIPTS})
    get_filename_component( _py_name ${_py_script} NAME_WE)
    file( READ ${_py_script} pytext)
    string(MD5 _md5 "${pytext}")
    set(TGT_NAME "${_py_name}-${_md5}" )
    set(PYLINT_OPTIONS --disable=no-member --disable=no-name-in-module
            --disable=invalid-unary-operand-type
            --disable=import-error
            --disable=no-method-argument
        )
    add_custom_target( ${TGT_NAME}
        COMMAND ${PYTHON_EXECUTABLE} -m pylint -E ${PYLINT_OPTIONS} ${_py_script}
        COMMENT "Running pylint on ${_py_script}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    add_dependencies(pylint ${TGT_NAME} )
endforeach( )

########################  DOCS ###############################################
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/devel/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation using Doxygen."
        VERBATIM)
else()
    message(STATUS "Doxygen needs to be installed to generate API docs")
endif()
