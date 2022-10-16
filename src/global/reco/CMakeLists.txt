cmake_minimum_required(VERSION 3.16)

get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
print_header(">>>>   P L U G I N :   ${PLUGIN_NAME}    <<<<")       # Fancy printing

# Function creates ${PLUGIN_NAME}_plugin and ${PLUGIN_NAME}_library targets
# Setting default includes, libraries and installation paths
plugin_add(${PLUGIN_NAME} WITH_STATIC_LIBRARY)

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp
# Then correctly sets sources for ${_name}_plugin and ${_name}_library targets
# Adds headers to the correct installation directory
plugin_glob_all(${PLUGIN_NAME})

# Find dependencies
find_package(EDM4HEP REQUIRED)
find_package(podio REQUIRED)
find_package(DD4hep REQUIRED)
find_package(ROOT REQUIRED COMPONENTS Core Tree Hist RIO EG)
find_package(EDM4EIC REQUIRED)
find_package(spdlog REQUIRED)
find_package(Boost REQUIRED)

# ACTS
find_package(Acts REQUIRED COMPONENTS Core PluginIdentification PluginTGeo PluginDD4hep)
set(Acts_VERSION_MIN "19.0.0")
set(Acts_VERSION "${Acts_VERSION_MAJOR}.${Acts_VERSION_MINOR}.${Acts_VERSION_PATCH}")
if(${Acts_VERSION} VERSION_LESS ${Acts_VERSION_MIN}
        AND NOT "${Acts_VERSION}" STREQUAL "9.9.9")
    message(FATAL_ERROR "Acts version ${Acts_VERSION_MIN} or higher required, but ${Acts_VERSION} found")
endif()

# Add include directories (works same as target_include_directories)
plugin_include_directories(${PLUGIN_NAME}
            PUBLIC
                ${CMAKE_CURRENT_LIST_DIR}
            SYSTEM PUBLIC
                ${podio_INCLUDE_DIR}
                ${EDM4HEP_INCLUDE_DIR}
                ${DD4hep_INCLUDE_DIRS}
                ${ROOT_INCLUDE_DIRS}
                ${EDM4EIC_INCLUDE_DIRS}
                ${Acts_INCLUDE_DIRS})


# Add libraries (works same as target_include_directories)
plugin_link_libraries(${PLUGIN_NAME}
        ${ROOT_LIBRARIES}
        dd4hep_library
        algorithms_tracking_library
        algorithms_reco_library
        EDM4HEP::edm4hep
        DD4hep::DDCore
        DD4hep::DDRec
        EDM4EIC::edm4eic
        ActsCore
        ActsPluginIdentification
        ActsPluginTGeo
        ActsPluginDD4hep
        spdlog::spdlog
        Boost::boost
        ${ROOT_EG_LIBRARY})

#target_link_libraries(${PLUGIN_NAME}_plugin
#        ${ROOT_LIBRARIES}
#        dd4hep_library
#        algorithms_tracking_library
#        EDM4HEP::edm4hep
#        DD4hep::DDCore
#        DD4hep::DDRec
#        EDM4EIC::edm4eic
#        ActsCore
#        ActsPluginIdentification
#        ActsPluginTGeo
#        ActsPluginDD4hep
#        spdlog::spdlog
#        Boost::boost)