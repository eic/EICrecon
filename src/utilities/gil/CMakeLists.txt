cmake_minimum_required(VERSION 3.16)

project(eicrecon_project LANGUAGES CXX)

# Find dependencies
find_package(JANA REQUIRED)
find_package(Threads REQUIRED)
find_package(EDM4HEP REQUIRED)
find_package(podio REQUIRED)
find_package(fmt REQUIRED)

# Compile all sources into executable
file(
  GLOB
  SOURCES
  *.cpp
  *.cc
  *.c
  *.hpp
  *.hh
  *.h)

set(INCLUDE_DIRS ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}
                 ${ROOT_INCLUDE_DIRS})
set(LINK_LIBRARIES
    ${ROOT_LIBRARIES}
    ${CMAKE_DL_LIBS}
    Threads::Threads
    EDM4HEP::edm4hep
    podio::podio
    podio::podioRootIO
    fmt::fmt)

# Define executable
add_executable(gil ${SOURCES})
target_include_directories(gil PUBLIC ${INCLUDE_DIRS})
target_link_libraries(gil ${LINK_LIBRARIES})
target_compile_definitions(
  gil PRIVATE EICRECON_APP_VERSION=${CMAKE_PROJECT_VERSION})

# Install executable
install(TARGETS gil DESTINATION bin)
