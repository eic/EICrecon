# Automatically set plugin name the same as the directory name
get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

# Function creates ${PLUGIN_NAME}_plugin and ${PLUGIN_NAME}_library targets
# Setting default includes, libraries and installation paths
plugin_add(${PLUGIN_NAME})

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp
# Then correctly sets sources for ${_name}_plugin and ${_name}_library targets
# Adds headers to the correct installation directory
plugin_glob_all(${PLUGIN_NAME})

# Get properties at configuration evaluation time. The `$<TARGET_PROPERTY`
# generator expression would include multiple additional paths from transitive
# dependencies such as PODIO and ROOT.
get_target_property(EDM4HEP_INTERFACE_INCLUDE_DIRECTORIES EDM4HEP::edm4hep INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(EDM4EIC_INTERFACE_INCLUDE_DIRECTORIES EDM4EIC::edm4eic INTERFACE_INCLUDE_DIRECTORIES)

# Get the datamodel version headers
set(EDM_VERSION_INCLUDES ${EDM4EIC_INTERFACE_INCLUDE_DIRECTORIES}/edm4eic/EDM4eicVersion.h)
if(${EDM4HEP_VERSION} VERSION_GREATER_EQUAL "0.9")
  list(APPEND EDM_VERSION_INCLUDES ${EDM4HEP_INTERFACE_INCLUDE_DIRECTORIES}/edm4hep/EDM4hepVersion.h)
endif()

# Generate the datamodel header files.
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/datamodel_glue.h ${CMAKE_CURRENT_BINARY_DIR}/datamodel_includes.h
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/make_datamodel_glue.py
        WORKING_DIR=${CMAKE_CURRENT_BINARY_DIR}
        EDM4HEP_INCLUDE_DIR=${EDM4HEP_INTERFACE_INCLUDE_DIRECTORIES}
        EDM4EIC_INCLUDE_DIR=${EDM4EIC_INTERFACE_INCLUDE_DIRECTORIES}
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/make_datamodel_glue.py
    DEPENDS ${EDM_VERSION_INCLUDES}
)

# Install datamodel_glue headers
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/datamodel_glue.h ${CMAKE_CURRENT_BINARY_DIR}/datamodel_includes.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/services/io/podio/)

# Add include directories (works same as target_include_directories)
plugin_include_directories(${PLUGIN_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# Add libraries (works same as target_include_directories)
plugin_link_libraries(${PLUGIN_NAME} fmt::fmt EDM4HEP::edm4hep EDM4HEP::edm4hepDict EDM4EIC::edm4eic EDM4EIC::edm4eic_utils podio::podioRootIO)

# Create a ROOT dictionary with the vector<edm4hep::XXXData> types defined. Without
# this, root will complain about not having a compiled CollectionProxy.
root_generate_dictionary(G__datamodel_vectors ${CMAKE_CURRENT_BINARY_DIR}/datamodel_includes.h MODULE ${PLUGIN_NAME}_plugin LINKDEF datamodel_LinkDef.h)

# Install root dictionaries made by PODIO
set(my_root_dict_files
    ${CMAKE_CURRENT_BINARY_DIR}/lib${PLUGIN_NAME}_plugin_rdict.pcm
    ${CMAKE_CURRENT_BINARY_DIR}/lib${PLUGIN_NAME}_plugin.rootmap
)
install(FILES ${my_root_dict_files} DESTINATION ${PLUGIN_OUTPUT_DIRECTORY})
