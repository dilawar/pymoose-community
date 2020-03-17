# Replicate Travis-CI building using docker
add_custom_target(travis
    COMMAND docker build -t bhallalab/travis:latest
        -f ${CMAKE_CURRENT_SOURCE_DIR}/docker/travis/Dockerfile .
    COMMENT "Replicating Travis-CI building using Docker."
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    VERBATIM)

add_custom_target(centos
    COMMAND docker build -t bhallalab/centos:latest
        -f ${CMAKE_CURRENT_SOURCE_DIR}/docker/centos/Dockerfile .
    COMMENT "Building moose on centos"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    VERBATIM)


# FIXME TESTS. These should not run by default. We need to fix them.
set(PYMOOSE_FIXME_TEST_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/devel/fixme)
file(GLOB PY_FIXME_TEST_SCRIPTS "${PYMOOSE_FIXME_TEST_DIRECTORY}/*.py" )
foreach( _test_script ${PY_FIXME_TEST_SCRIPTS} )
    get_filename_component( _name_we ${_test_script} NAME_WE)
    set(_test_name "alpha_${_name_we}")
    add_test( NAME ${_test_name}
        COMMAND ${PYTHON_EXECUTABLE} ${_test_script}
        CONFIGURATIONS alpha
	WORKING_DIRECTORY ${PYMOOSE_ALPHA_TEST_DIRECTORY}
        )
     set_tests_properties( ${_test_name}
         PROPERTIES ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/python"
        )
endforeach( )

# Regression and github issues. These should not run by default.
set(PYMOOSE_ISSUES_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/devel/issues)
file(GLOB PY_ISSUES_SCRIPTS "${PYMOOSE_ISSUES_DIRECTORY}/*.py" )
foreach(_test_script ${PY_ISSUES_SCRIPTS})
    get_filename_component( _test_name ${_test_script} NAME_WE)
    add_test(NAME ${_test_name}
        COMMAND ${PYTHON_EXECUTABLE} ${_test_script}
	CONFIGURATIONS Devel
	WORKING_DIRECTORY ${PYMOOSE_ISSUES_DIRECTORY}
        )
     set_tests_properties(${_test_name}
         PROPERTIES ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/python"
        )
endforeach()
