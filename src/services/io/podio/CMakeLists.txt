# For podio < 1.3, generate datamodel files with Python script
if(podio_VERSION VERSION_LESS "1.3" OR JANA_VERSION VERSION_LESS "2.4.3")
  message(
    STATUS "Using legacy Python-based datamodel glue for podio ${podio_VERSION}"
  )

  # Get properties at configuration evaluation time. The `$<TARGET_PROPERTY`
  # generator expression would include multiple additional paths from transitive
  # dependencies such as PODIO and ROOT.
  get_target_property(EDM4HEP_INTERFACE_INCLUDE_DIRECTORIES EDM4HEP::edm4hep
                      INTERFACE_INCLUDE_DIRECTORIES)
  get_target_property(EDM4EIC_INTERFACE_INCLUDE_DIRECTORIES EDM4EIC::edm4eic
                      INTERFACE_INCLUDE_DIRECTORIES)

  # Get the datamodel version headers
  set(EDM_VERSION_INCLUDES
      ${EDM4EIC_INTERFACE_INCLUDE_DIRECTORIES}/edm4eic/EDM4eicVersion.h)
  if(${EDM4HEP_VERSION} VERSION_GREATER_EQUAL "0.9")
    list(APPEND EDM_VERSION_INCLUDES
         ${EDM4HEP_INTERFACE_INCLUDE_DIRECTORIES}/edm4hep/EDM4hepVersion.h)
  endif()

  # Get relative source dir
  file(RELATIVE_PATH DATAMODEL_RELATIVE_PATH ${PROJECT_SOURCE_DIR}/src
       ${CMAKE_CURRENT_SOURCE_DIR})

  # Generate the datamodel header files.
  add_custom_command(
    OUTPUT
      ${PROJECT_BINARY_DIR}/include/${DATAMODEL_RELATIVE_PATH}/datamodel_glue_legacy.h
      ${PROJECT_BINARY_DIR}/include/${DATAMODEL_RELATIVE_PATH}/datamodel_includes_legacy.h
    COMMAND
      python3 ${CMAKE_CURRENT_SOURCE_DIR}/make_datamodel_glue.py
      WORKING_DIR=${PROJECT_BINARY_DIR}/include/${DATAMODEL_RELATIVE_PATH}
      EDM4HEP_INCLUDE_DIR=${EDM4HEP_INTERFACE_INCLUDE_DIRECTORIES}
      EDM4EIC_INCLUDE_DIR=${EDM4EIC_INTERFACE_INCLUDE_DIRECTORIES}
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/make_datamodel_glue.py
    DEPENDS ${EDM_VERSION_INCLUDES}
    COMMENT "Create legacy datamodel glue headers")
  add_custom_target(
    podio_datamodel_glue
    DEPENDS
      ${PROJECT_BINARY_DIR}/include/${DATAMODEL_RELATIVE_PATH}/datamodel_glue_legacy.h
    COMMENT "Create legacy datamodel glue headers")

  # Install datamodel_glue headers
  install(
    FILES
      ${PROJECT_BINARY_DIR}/include/${DATAMODEL_RELATIVE_PATH}/datamodel_glue_legacy.h
      ${PROJECT_BINARY_DIR}/include/${DATAMODEL_RELATIVE_PATH}/datamodel_includes_legacy.h
    DESTINATION
      ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/${DATAMODEL_RELATIVE_PATH})

else()
  message(STATUS "Using modern C++ datamodel glue for podio ${podio_VERSION}")

  # Just create an interface target
  add_custom_target(podio_datamodel_glue
                    COMMENT "Using modern datamodel glue headers")

endif()
# Note that compatibility and modern datamodel glue headers are installed by the
# logic in plugin_glob_all. Automatically set plugin name the same as the
# directory name
get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

# Function creates ${PLUGIN_NAME}_plugin and ${PLUGIN_NAME}_library targets
# Setting default includes, libraries and installation paths
plugin_add(${PLUGIN_NAME})

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp Then
# correctly sets sources for ${_name}_plugin and ${_name}_library targets Adds
# headers to the correct installation directory
plugin_glob_all(${PLUGIN_NAME})

# Add include directories (works same as target_include_directories)
plugin_include_directories(${PLUGIN_NAME} PRIVATE ${PROJECT_BINARY_DIR}/include)

# Add libraries (works same as target_include_directories)
plugin_link_libraries(
  ${PLUGIN_NAME}
  fmt::fmt
  EDM4HEP::edm4hep
  EDM4HEP::edm4hepDict
  EDM4EIC::edm4eic
  EDM4EIC::edm4eic_utils
  podio::podioRootIO
  log_library)
