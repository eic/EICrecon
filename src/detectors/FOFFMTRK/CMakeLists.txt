# Automatically set plugin name the same as the directory name
# Don't forget string(REPLACE " " "_" PLUGIN_NAME ${PLUGIN_NAME}) if this dir has spaces in its name
get_filename_component(PLUGIN_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

print_header(">>>>   P L U G I N :   ${PLUGIN_NAME}    <<<<")       # Fancy printing

# Function creates ${PLUGIN_NAME}_plugin and ${PLUGIN_NAME}_library targets
# Setting default includes, libraries and installation paths
plugin_add(${PLUGIN_NAME})

# Find dependencies
plugin_add_dd4hep(${PLUGIN_NAME})
plugin_add_acts(${PLUGIN_NAME})
plugin_add_cern_root(${PLUGIN_NAME})
plugin_add_event_model(${PLUGIN_NAME})

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp
# Then correctly sets sources for ${_name}_plugin and ${_name}_library targets
# Adds headers to the correct installation directory
plugin_glob_all(${PLUGIN_NAME})

# Add include directories (works same as target_include_directories)
# plugin_include_directories(${PLUGIN_NAME} SYSTEM PUBLIC ... )

# Add libraries
# (same as target_include_directories but for both plugin and library)
plugin_link_libraries(${PLUGIN_NAME} algorithms_digi_library algorithms_tracking_library)oot/root-v6-26-04/lib/libCore.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libImt.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libRIO.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libNet.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libHist.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libGraf.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libGraf3d.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libGpad.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libROOTDataFrame.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libTree.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libTreePlayer.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libRint.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libPostscript.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libMatrix.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libPhysics.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libMathCore.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libThread.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libMultiProc.so;/home/romanov/eic/soft/root/root-v6-26-04/lib/libROOTVecOps.so