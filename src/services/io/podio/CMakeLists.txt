cmake_minimum_required(VERSION 3.16)

# Automatically set plugin name the same as the directory name
get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

project(datamodel)

# Function creates ${PLUGIN_NAME}_plugin and ${PLUGIN_NAME}_library targets
# Setting default includes, libraries and installation paths
eicrecon_add(${PLUGIN_NAME})

# Find dependencies
eicrecon_add_dd4hep(${PLUGIN_NAME})
eicrecon_add_cern_root(${PLUGIN_NAME})
eicrecon_add_event_model(${PLUGIN_NAME})

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp
# Then correctly sets sources for ${_name}_plugin and ${_name}_library targets
# Adds headers to the correct installation directory
eicrecon_glob_all(${PLUGIN_NAME})

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
    COMMAND python3 ${PROJECT_SOURCE_DIR}/make_datamodel_glue.py
        WORKING_DIR=${CMAKE_CURRENT_BINARY_DIR}
        EDM4HEP_INCLUDE_DIR=${EDM4HEP_INTERFACE_INCLUDE_DIRECTORIES}
        EDM4EIC_INCLUDE_DIR=${EDM4EIC_INTERFACE_INCLUDE_DIRECTORIES}
    MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/make_datamodel_glue.py
    DEPENDS ${EDM_VERSION_INCLUDES}
)

# Add include directories (works same as target_include_directories)
eicrecon_include_directories(${PLUGIN_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# Create a ROOT dictionary with the datamodel types defined. Without
# this, root will complain about not having a compiled CollectionProxy.
root_generate_dictionary(G__${PLUGIN_NAME} ${datamodel_BINARY_DIR}/datamodel_includes.h MODULE ${PLUGIN_NAME}_library LINKDEF datamodel_LinkDef.h)

# Install root dictionaries made by PODIO
set(${PLUGIN_NAME}_dict_files
    ${PROJECT_BINARY_DIR}/lib${PLUGIN_NAME}_library_rdict.pcm
    ${PROJECT_BINARY_DIR}/lib${PLUGIN_NAME}_library.rootmap
)
install(FILES ${${PLUGIN_NAME}_dict_files} DESTINATION ${PLUGIN_OUTPUT_DIRECTORY})
