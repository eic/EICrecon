cmake_minimum_required(VERSION 3.16)


project(eicrecon_project)

# Find dependencies
find_package(JANA    REQUIRED)
find_package(Threads REQUIRED)
find_package(fmt     REQUIRED)
find_package(ROOT REQUIRED COMPONENTS Core Tree Hist RIO EG)

# Compile all sources into executable
set(SOURCES
  eicrecon_cli.cpp
  eicrecon.cc
  parser.cc
  eicrecon_cli.h
  parser.h
  print_info.h
)

set( INCLUDE_DIRS  ${PROJECT_BINARY_DIR} ${EICRECON_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR} ${JANA_INCLUDE_DIR} ${ROOT_INCLUDE_DIRS})
set( LINK_LIBRARIES ${JANA_LIB} ${ROOT_LIBRARIES} ${CMAKE_DL_LIBS} Threads::Threads podio::podio podio::podioRootIO)

# Define executable
add_executable( eicrecon ${SOURCES} )
target_include_directories( eicrecon PUBLIC ${INCLUDE_DIRS})
target_link_libraries(eicrecon ${LINK_LIBRARIES} DD4hep::DDParsers fmt::fmt)
target_compile_definitions(eicrecon PRIVATE EICRECON_APP_VERSION=${CMAKE_PROJECT_VERSION})
install(TARGETS eicrecon DESTINATION bin)

# Define tests
find_package(Catch2 3)
if(Catch2_FOUND)
  add_executable( param_parser_test param_parser_test.cc parser.cc parser.h)
  target_include_directories( param_parser_test PUBLIC ${INCLUDE_DIRS})
  target_link_libraries(param_parser_test ${LINK_LIBRARIES} DD4hep::DDParsers fmt::fmt Catch2::Catch2WithMain)
  install(TARGETS param_parser_test DESTINATION bin)
else()
    message(STATUS "Catch2 is not found. Skipping bin/eicrecon tests...")
endif()
