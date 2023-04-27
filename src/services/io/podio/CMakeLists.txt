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

set(EDM4EIC_INCLUDE_DIR ${EDM4EIC_DIR}/../../include)
set(fmt_INCLUDE_DIR ${fmt_DIR}/../../../include)
set(INCLUDE_DIRS ${podio_INCLUDE_DIR} ${EDM4HEP_INCLUDE_DIR} ${DD4hep_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${EDM4EIC_INCLUDE_DIR} ${fmt_INCLUDE_DIR})

# Function creates ${PLUGIN_NAME}_plugin and ${PLUGIN_NAME}_library targets
# Setting default includes, libraries and installation paths
plugin_add(${PLUGIN_NAME} )

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp
# Then correctly sets sources for ${_name}_plugin and ${_name}_library targets
# Adds headers to the correct installation directory
plugin_glob_all(${PLUGIN_NAME})

# Generate the datamodel header files.
add_custom_command(
        OUTPUT ${podio_BINARY_DIR}/datamodel_glue.h ${podio_BINARY_DIR}/datamodel_includes.h
        COMMAND python3 ${PROJECT_SOURCE_DIR}/make_datamodel_glue.py WORKING_DIR=${podio_BINARY_DIR} EDM4HEP_INCLUDE_DIR=${EDM4HEP_INCLUDE_DIR} EDM4EIC_INCLUDE_DIR=${EDM4EIC_INCLUDE_DIR}
)

set(LINK_LIBRARIES EDM4HEP::edm4hep EDM4HEP::edm4hepDict EDM4EIC::edm4eic EDM4EIC::edm4eic_utils podio::podioRootIO)

# Add include directories (works same as target_include_directories)
message(STATUS "fmt_INCLUDE_DIR=${fmt_INCLUDE_DIR}")
plugin_include_directories(${PLUGIN_NAME} SYSTEM PUBLIC ${INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR} ${EICRECON_SOURCE_DIR}/src)

# Add libraries (works same as target_include_directories)
plugin_link_libraries(${PLUGIN_NAME} ${LINK_LIBRARIES})

# Create a ROOT dictionary with the vector<edm4hep::XXXData> types defined. Without
# this, root will complain about not having a compiled CollectionProxy.
root_generate_dictionary(G__datamodel_vectors ${podio_BINARY_DIR}/datamodel_includes.h MODULE ${PLUGIN_NAME}_plugin LINKDEF datamodel_LinkDef.h)

# Install root dictionaries made by PODIO
set(my_root_dict_files
        ${PROJECT_BINARY_DIR}/lib${PLUGIN_NAME}_plugin_rdict.pcm
        ${PROJECT_BINARY_DIR}/lib${PLUGIN_NAME}_plugin.rootmap
        )
install(FILES ${my_root_dict_files} DESTINATION ${PLUGIN_OUTPUT_DIRECTORY})
