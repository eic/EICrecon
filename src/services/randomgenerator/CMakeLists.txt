cmake_minimum_required(VERSION 3.9)
cmake_policy(SET CMP0074 NEW)  # use the policy to look for <package>_ROOT envar

#compile RandGen_driver
project("RandGen_driver")

add_executable("${PROJECT_NAME}" "RandGen_driver.cpp")

install(TARGETS "${PROJECT_NAME}" DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
install(FILES RandGen_driver.cpp DESTINATION src)

