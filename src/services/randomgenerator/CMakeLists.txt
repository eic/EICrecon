cmake_minimum_required(VERSION 3.16)

get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

project("randomgenerator")

add_executable("${PROJECT_NAME}" "randomGenerator.cc")

install(TARGETS "${PROJECT_NAME}" DESTINATION "${CMAKE_SOURCE_DIR}/tests")
