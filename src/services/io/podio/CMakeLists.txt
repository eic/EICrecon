cmake_minimum_required(VERSION 3.16)


# Automatically set plugin name the same as the directory name
get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

project(podio)

# Find dependencies (JANA and spdlog automatically added already)
find_package(EDM4HEP REQUIRED)
find_package(EDM4EIC REQUIRED)
find_package(podio REQUIRED)
find_package(ROOT REQUIRED COMPONENTS Core Tree Hist RIO EG)
find_package(fmt REQUIRED)
if(NOT DD4hep_FOUND)
    find_package(DD4hep REQUIRED)
endif()

# FIXME should use $<TARGET_PROPERTY:edm4eic,INTERFACE_INCLUDE_DIRECTORIES>
# Also, we could just export this in the EDM4eic cmake config like EDM4hep
set(EDM4EIC_INCLUDE_DIR ${EDM4EIC_DIR}/../../include)

# Function creates ${PLUGIN_NAME}_plugin and ${PLUGIN_NAME}_library targets
# Setting default includes, libraries and installation paths
plugin_add(${PLUGIN_NAME})

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp
# Then correctly sets sources for ${_name}_plugin and ${_name}_library targets
# Adds headers to the correct installation directory
plugin_glob_all(${PLUGIN_NAME})

# Generate the datamodel header files.
file(GLOB EDM4HEP_DEPENDS ${EDM4HEP_INCLUDE_DIR}/edm4hep/*.h)
file(GLOB EDM4EIC_DEPENDS ${EDM4EIC_INCLUDE_DIR}/edm4eic/*.h)
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/datamodel_glue.h ${CMAKE_CURRENT_BINARY_DIR}/datamodel_includes.h
        COMMAND python3 ${PROJECT_SOURCE_DIR}/make_datamodel_glue.py WORKING_DIR=${CMAKE_CURRENT_BINARY_DIR} EDM4HEP_INCLUDE_DIR=${EDM4HEP_INCLUDE_DIR} EDM4EIC_INCLUDE_DIR=${EDM4EIC_INCLUDE_DIR}
        MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/make_datamodel_glue.py
	DEPENDS ${EDM4HEP_DEPENDS} ${EDM4EIC_DEPENDS}
)

# Add include directories (works same as target_include_directories)
plugin_include_directories(${PLUGIN_NAME} SYSTEM PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

# Add libraries (works same as target_include_directories)
plugin_link_libraries(${PLUGIN_NAME} fmt::fmt EDM4HEP::edm4hep EDM4HEP::edm4hepDict EDM4EIC::edm4eic EDM4EIC::edm4eic_utils podio::podioRootIO)

# Create a ROOT dictionary with the vector<edm4hep::XXXData> types defined. Without
# this, root will complain about not having a compiled CollectionProxy.
root_generate_dictionary(G__datamodel_vectors ${podio_BINARY_DIR}/datamodel_includes.h MODULE ${PLUGIN_NAME}_plugin LINKDEF datamodel_LinkDef.h)

# Install root dictionaries made by PODIO
set(my_root_dict_files
        ${PROJECT_BINARY_DIR}/lib${PLUGIN_NAME}_plugin_rdict.pcm
        ${PROJECT_BINARY_DIR}/lib${PLUGIN_NAME}_plugin.rootmap
        )
install(FILES ${my_root_dict_files} DESTINATION ${PLUGIN_OUTPUT_DIRECTORY})
