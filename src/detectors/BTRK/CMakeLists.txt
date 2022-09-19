# Automatically set plugin name the same as the directory name
# Don't forget string(REPLACE " " "_" PLUGIN_NAME ${PLUGIN_NAME}) if this dir has spaces in its name
get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

print_header(">>>>   P L U G I N :   ${PLUGIN_NAME}    <<<<")       # Fancy printing

# Function creates ${PLUGIN_NAME}_plugin and ${PLUGIN_NAME}_library targets
# Setting default includes, libraries and installation paths
plugin_add(${PLUGIN_NAME})

# Find dependencies
find_package(JANA REQUIRED)
find_package(EDM4HEP REQUIRED)
find_package(podio REQUIRED)
find_package(DD4hep REQUIRED)
find_package(ROOT REQUIRED COMPONENTS Core Tree Hist RIO EG)
find_package(EDM4EIC REQUIRED)
find_package(spdlog REQUIRED)

# ACTS
find_package(Acts REQUIRED COMPONENTS Core PluginIdentification PluginTGeo PluginDD4hep)
message(STATUS "Acts_FIND_COMPONENTS = ${Acts_FIND_COMPONENTS}")
set(Acts_VERSION_MIN "19.0.0")
set(Acts_VERSION "${Acts_VERSION_MAJOR}.${Acts_VERSION_MINOR}.${Acts_VERSION_PATCH}")
if(${Acts_VERSION} VERSION_LESS ${Acts_VERSION_MIN}
        AND NOT "${Acts_VERSION}" STREQUAL "9.9.9")
    message(FATAL_ERROR "Acts version ${Acts_VERSION_MIN} or higher required, but ${Acts_VERSION} found")
endif()

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp
# Then correctly sets sources for ${_name}_plugin and ${_name}_library targets
# Adds headers to the correct installation directory
plugin_glob_all(${PLUGIN_NAME})

# Add include directories
# (same as target_include_directories but for both plugin and library)
plugin_include_directories(${PLUGIN_NAME} SYSTEM PUBLIC ${podio_INCLUDE_DIR} ${EDM4EIC_INCLUDE_DIR} ${EDM4HEP_INCLUDE_DIR} ${DD4hep_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} )

message(STATUS "{ROOT_LIBRARIES} ${ROOT_LIBRARIES}")

#string(REPLACE ";" " " ROOT_LIBRARIES ${ROOT_LIBRARIES})
#
#message(STATUS "{ROOT_LIBRARIES}2 ${ROOT_LIBRARIES}")

# Add libraries
# (same as target_include_directories but for both plugin and library)
plugin_link_libraries(${PLUGIN_NAME}
            ${JANA_LIB}
            ${ROOT_LIBRARIES}
            ActsCore
            ActsPluginIdentification
            ActsPluginTGeo
            ActsPluginDD4hep
            EDM4EIC::edm4eic
            algorithms_digi_library
            algorithms_tracking_library
            tracking_library
            digi_library
            ROOT::EG
        )
plugin_link_libraries(${PLUGIN_NAME} ${JANA_LIB} ${ROOT_LIBRARIES} ActsCore ActsPluginIdentification ActsPluginTGeo ActsPluginDD4hep EDM4EIC::edm4eic algorithms_digi_library algorithms_tracking_library ROOT::EG)
#target_link_libraries(${PLUGIN_NAME}_plugin ActsCore)

#/home/romanov/eic/soft/root/root-v6-26-04/lib/libCore.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libImt.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libRIO.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libNet.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libHist.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libGraf.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libGraf3d.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libGpad.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libROOTDataFrame.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libTree.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libTreePlayer.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libRint.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libPostscript.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libMatrix.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libPhysics.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libMathCore.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libThread.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libMultiProc.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libROOTVecOps.so