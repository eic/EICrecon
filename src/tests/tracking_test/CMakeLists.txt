# Automatically set plugin name the same as the directory name
# Don't forget string(REPLACE " " "_" PLUGIN_NAME ${PLUGIN_NAME}) if this dir has spaces in its name
get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
print_header(">>>>   P L U G I N :   ${PLUGIN_NAME}    <<<<")       # Fancy printing

# Function creates ${PLUGIN_NAME}_plugin and ${PLUGIN_NAME}_library targets
# Setting default includes, libraries and installation paths
plugin_add(${PLUGIN_NAME})

# Find dependencies
find_package(JANA REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(EDM4HEP REQUIRED)
find_package(podio REQUIRED)
find_package(DD4hep REQUIRED)
find_package(ROOT REQUIRED COMPONENTS Core Tree Hist RIO EG)
find_package(EDM4EIC)

# ACTS
find_package(Acts REQUIRED COMPONENTS Core PluginIdentification PluginTGeo PluginDD4hep)
set(Acts_VERSION_MIN "19.0.0")
set(Acts_VERSION "${Acts_VERSION_MAJOR}.${Acts_VERSION_MINOR}.${Acts_VERSION_PATCH}")
if(${Acts_VERSION} VERSION_LESS ${Acts_VERSION_MIN}
        AND NOT "${Acts_VERSION}" STREQUAL "9.9.9")
    message(FATAL_ERROR "Acts version ${Acts_VERSION_MIN} or higher required, but ${Acts_VERSION} found")
endif()

set(Acts_INCLUDE_DIR "${Acts_DIR}/../../../include/")
message("{Acts_INCLUDE_DIR} ${Acts_INCLUDE_DIR}")


# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp
# Then correctly sets sources for ${_name}_plugin and ${_name}_library targets
# Adds headers to the correct installation directory
plugin_glob_all(${PLUGIN_NAME})

# Add include directories
# (same as target_include_directories but for both plugin and library)
plugin_include_directories(${PLUGIN_NAME}
        SYSTEM PUBLIC
            ${EDM4EIC_INCLUDE_DIR}
            ${podio_INCLUDE_DIR}
            ${EDM4HEP_INCLUDE_DIR}
            ${DD4hep_INCLUDE_DIRS}
            ${ROOT_INCLUDE_DIRS}
            ${Acts_INCLUDE_DIRS})

# Add libraries
# (same as target_include_directories but for both plugin and library)
plugin_link_libraries(${PLUGIN_NAME} Eigen3::Eigen ${ROOT_LIBRARIES} ActsCore EDM4EIC::edm4eic)
