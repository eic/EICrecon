cmake_minimum_required(VERSION 3.16)

# Automatically set plugin name the same as the directory name
# Don't forget string(REPLACE " " "_" PLUGIN_NAME ${PLUGIN_NAME}) if this dir has spaces in its name
get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

# Function creates ${PLUGIN_NAME}_plugin and ${PLUGIN_NAME}_library targets
# Setting default includes, libraries and installation paths
plugin_add(${PLUGIN_NAME})

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp
# Then correctly sets sources for ${_name}_plugin and ${_name}_library targets
# Adds headers to the correct installation directory
plugin_glob_all(${PLUGIN_NAME})

# Find dependencies
plugin_add_dd4hep(${PLUGIN_NAME})
plugin_add_acts(${PLUGIN_NAME})
plugin_add_event_model(${PLUGIN_NAME})

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp
# Then correctly sets sources for ${_name}_plugin and ${_name}_library targets
# Adds headers to the correct installation directory
plugin_glob_all(${PLUGIN_NAME})

# Add include directories (works same as target_include_directories)
# plugin_include_directories(${PLUGIN_NAME} SYSTEM PUBLIC ... )

# Add libraries
# (same as target_include_directories but for both plugin and library)
plugin_link_libraries(${PLUGIN_NAME} podio::podio podio::podioRootIO algorithms_digi_library algorithms_tracking_library)

#
## Add include directories (works same as target_include_directories)
#plugin_include_directories(${PLUGIN_NAME}
#        PUBLIC
#        ${CMAKE_CURRENT_LIST_DIR}
#        SYSTEM PUBLIC
#        ${podio_INCLUDE_DIR}
#        ${EDM4HEP_INCLUDE_DIR}
#        ${DD4hep_INCLUDE_DIRS}
#        ${ROOT_INCLUDE_DIRS}
#        ${EDM4EIC_INCLUDE_DIRS})
#
#
## Add libraries (works same as target_include_directories)
#plugin_link_libraries(${PLUGIN_NAME} EDM4HEP::edm4hep  EDM4EIC::edm4eic ActsCore ActsPluginDD4hep ActsPluginJson ActsPluginTGeo spdlog::spdlog algorithms_tracking_library)
#
#target_link_libraries(${PLUGIN_NAME}_plugin EDM4HEP::edm4hep  EDM4EIC::edm4eic ActsCore ActsPluginDD4hep ActsPluginJson ActsPluginTGeo spdlog::spdlog algorithms_tracking_library)
#
#get_target_property(ACTSPLUGINLIBS ${PLUGIN_NAME}_plugin LINK_LIBRARIES)
#message(STATUS "LINK_LIBRARIES = ${ACTSPLUGINLIBS}")
#
#
##LINK_LIBRARIES = JANA::jana2;spdlog::spdlog;EDM4HEP::edm4hep;;EDM4EIC::edm4eic;ActsCore;ActsPluginDD4hep;ActsPluginJson;ActsPluginTGeo;spdlog::spdlog;algorithms_tracking_library
##LINK_LIBRARIES = JANA::jana2;spdlog::spdlog;EDM4HEP::edm4hep;;EDM4EIC::edm4eic;ActsCore;ActsPluginDD4hep;ActsPluginJson;ActsPluginTGeo;spdlog::spdlog;algorithms_tracking_library;
##                                            EDM4HEP::edm4hep;;EDM4EIC::edm4eic;ActsCore;ActsPluginDD4hep;ActsPluginJson;ActsPluginTGeo;spdlog::spdlog;algorithms_tracking_library
#target_link_libraries(${PLUGIN_NAME}_plugin
#        Eigen3::Eigen
#        EDM4HEP::edm4hep
#
#        EDM4EIC::edm4eic
#        ActsCore
#        ActsPluginDD4hep
#        ActsPluginJson
#        ActsPluginTGeo
#        spdlog::spdlog
#        algorithms_tracking_library)
