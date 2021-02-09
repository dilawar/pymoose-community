#
# This script install wheel. 
#

file(GLOB WHEEL_FILES "*.whl")

foreach(_wheel_file ${WHEEL_FILES})
    message(STATUS "[INSTALL] Wheel ${_wheel_file}")
    execute_process(COMMAND ${PYTHON_EXECUTABLE} -m pip install ${_wheel_file})
endforeach()

