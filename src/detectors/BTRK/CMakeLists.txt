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
find_package(EICD REQUIRED)
find_package(spdlog REQUIRED)

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp
# Then correctly sets sources for ${_name}_plugin and ${_name}_library targets
# Adds headers to the correct installation directory
plugin_glob_all(${PLUGIN_NAME})

# Add include directories
# (same as target_include_directories but for both plugin and library)
plugin_include_directories(${PLUGIN_NAME} SYSTEM PUBLIC ${podio_INCLUDE_DIR} ${EICD_INCLUDE_DIR} ${EDM4HEP_INCLUDE_DIR} ${DD4hep_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} )

message(STATUS "{ROOT_LIBRARIES} ${ROOT_LIBRARIES}")

#string(REPLACE ";" " " ROOT_LIBRARIES ${ROOT_LIBRARIES})
#
#message(STATUS "{ROOT_LIBRARIES}2 ${ROOT_LIBRARIES}")

# Add libraries
# (same as target_include_directories but for both plugin and library)
plugin_link_libraries(${PLUGIN_NAME} ${JANA_LIB} ${ROOT_LIBRARIES} spdlog::spdlog EICD::eicd digi_library tracking_library)

target_link_libraries(${PLUGIN_NAME}_plugin  ${ROOT_LIBRARIES} /home/romanov/eic/soft/root/root-v6-26-04/lib/libEG.so)