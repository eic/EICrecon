cmake_minimum_required(VERSION 3.9)
cmake_policy(SET CMP0074 NEW)  # use the policy to look for <package>_ROOT envar

print_header(">>>>   E X E C U T A B L E :   algorithms    <<<<")       # Fancy printing

project(eicrecon_project)

# Find dependencies
find_package(JANA    REQUIRED)
find_package(EDM4HEP REQUIRED)
find_package(podio   REQUIRED)
find_package(ROOT    REQUIRED)
find_package(DD4hep  REQUIRED)
find_package(fmt     REQUIRED)


set(SERVICES_LIBRARIES
        podio_library EDM4HEP::edm4hep EDM4HEP::edm4hepDict podio::podioRootIO
        dd4hep_library ${DD4hep_LIBRARIES} fmt::fmt ${DETECTOR_LIBS_EPIC}  ${DETECTOR_LIBS_IP6}
        )
set(DETECTOR_LIBRARIES
        BEMC_library
        )


# Compile all sources into executable
file(GLOB SOURCES *.cpp *.cc *.c  *.hpp *.hh *.h)

set( INCLUDE_DIRS  ${PROJECT_BINARY_DIR} ${CMAKE_SOURCE_DIR} ${PROJECT_SOURCE_DIR} ${JANA_INCLUDE_DIR} ${ROOT_INCLUDE_DIRS})
set( LINK_LIBRARIES ${JANA_LIB} ${SERVICES_LIBRARIES} ${DETECTOR_LIBRARIES} ${CMAKE_DL_LIBS})

# Define executable
add_executable( eicrecon ${SOURCES} )
target_include_directories( eicrecon PUBLIC ${INCLUDE_DIRS})
target_link_libraries(eicrecon ${LINK_LIBRARIES})

# Install executable
install(TARGETS eicrecon DESTINATION bin)


