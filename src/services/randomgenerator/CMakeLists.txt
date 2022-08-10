cmake_minimum_required(VERSION 3.16)

get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
print_header(">>>>   P L U G I N :   ${PLUGIN_NAME}    <<<<")       # Fancy printing

#compile RandGen_driver
project("RandGen_driver")

add_executable("${PROJECT_NAME}" "RandGen_driver.cpp")

install(TARGETS "${PROJECT_NAME}" DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
#install(FILES RandGen_driver.cpp DESTINATION src)

