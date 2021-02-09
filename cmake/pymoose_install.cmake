#
# This script install wheel. 
#

file(GLOB WHEEL_FILES "*.whl")
find_package(Python3 COMPONENTS Interpreter REQUIRED)

foreach(_wheel_file ${WHEEL_FILES})
    message(STATUS "[INSTALL] installing ${_wheel_file}")
    message(STATUS "[INSTALL] Using python executable ${Python3_EXECUTABLE}")
    execute_process(COMMAND ${Python3_EXECUTABLE} -m pip install ${_wheel_file}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
endforeach()

