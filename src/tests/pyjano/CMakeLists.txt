

project(pyjano_playroom)

set(OUTPUT_FILE_NAME "eicrecon_path.py")
message(STATUS "generating ${OUTPUT_FILE_NAME}")
message(STATUS "CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH}")
message(STATUS "CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")
#file(WRITE ${OUTPUT_FILE_NAME} "cmake_source_root = '${CMAKE_INSTALL_PREFIX}'")
