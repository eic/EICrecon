cmake_minimum_required(VERSION 3.16)


# Automatically set plugin name the same as the directory name
get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
print_header(">>>>   P L U G I N :   ${PLUGIN_NAME}    <<<<")       # Fancy printing

project(podio_project)

# Find dependencies (JANA and spdlog automatically added already)
find_package(EDM4EIC REQUIRED)
find_package(podio REQUIRED)
find_package(ROOT REQUIRED COMPONENTS Core Tree Hist RIO EG)
find_package(fmt REQUIRED)

set(EDM4EIC_INCLUDE_DIR ${EDM4EIC_DIR}/../../include)
set(fmt_INCLUDE_DIR ${fmt_DIR}/../../../include)
set(INCLUDE_DIRS ${fmt_INCLUDE_DIR})

# Function creates ${PLUGIN_NAME}_plugin and ${PLUGIN_NAME}_library targets
# Setting default includes, libraries and installation paths
plugin_add(${PLUGIN_NAME} )

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp
# Then correctly sets sources for ${_name}_plugin and ${_name}_library targets
# Adds headers to the correct installation directory
plugin_glob_all(${PLUGIN_NAME})

# Generate the datamodel header files.
# This would be better served with the add_custom_command feature of cmake,
# but that never really worked quite right. Thus, we just force-run it
# whenever cmake is run.
execute_process (
        COMMAND python3 ${PROJECT_SOURCE_DIR}/make_datamodel_glue.py WORKING_DIR=${PROJECT_BINARY_DIR} EDM4HEP_INCLUDE_DIR=${EDM4HEP_INCLUDE_DIR} EDM4EIC_INCLUDE_DIR=${EDM4EIC_INCLUDE_DIR}
)

# Create a ROOT dictionary with the vector<edm4hep::XXXData> types defined. Without
# this, root will complain about not having a compiled CollectionProxy.
include_directories( ${INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
root_generate_dictionary(G__datamodel_vectors datamodel_includes.h LINKDEF datamodel_LinkDef.h)
target_sources(${PLUGIN_NAME}_plugin PRIVATE datamodel_includes.h G__datamodel_vectors.cxx)

plugin_add_dd4hep(${PLUGIN_NAME})
plugin_add_event_model(${PLUGIN_NAME})

# Additional libraries
plugin_link_libraries(${PLUGIN_NAME} podio::podioRootIO log_library)

# Add include directories (works same as target_include_directories)
message(STATUS "fmt_INCLUDE_DIR=${fmt_INCLUDE_DIR}")
plugin_include_directories(${PLUGIN_NAME} SYSTEM PUBLIC ${INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src)

# Install root dictionaries made by PODIO
set(my_root_dict_files
        ${PROJECT_BINARY_DIR}/libdatamodel_vectors_rdict.pcm
        ${PROJECT_BINARY_DIR}/libdatamodel_vectors.rootmap
        )
install(FILES ${my_root_dict_files} DESTINATION ${PLUGIN_OUTPUT_DIRECTORY})
