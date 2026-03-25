# Compile all sources into executable
file(GLOB SOURCES *.cc *.h)

# Define executable
add_executable(eicrecon ${SOURCES})

# Set include directories
target_include_directories(eicrecon PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_include_directories(eicrecon SYSTEM PRIVATE ${JANA_INCLUDE_DIR}
                                                   ${ROOT_INCLUDE_DIRS})

# Link libraries
target_link_libraries(eicrecon PRIVATE ${JANA_LIB} ROOT::Core)

# Set compile definitions
target_compile_definitions(
  eicrecon PRIVATE EICRECON_APP_VERSION=${CMAKE_PROJECT_VERSION})

# Install executable
install(TARGETS eicrecon DESTINATION ${CMAKE_INSTALL_BINDIR})
