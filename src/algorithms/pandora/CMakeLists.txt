set(PLUGIN_NAME "algorithms_pandora")

# Function creates ${PLUGIN_NAME}_plugin and ${PLUGIN_NAME}_library targets
# Setting default includes, libraries and installation paths
plugin_add(${PLUGIN_NAME} WITH_SHARED_LIBRARY WITHOUT_PLUGIN)

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp Then
# correctly sets sources for ${_name}_plugin and ${_name}_library targets Adds
# headers to the correct installation directory
plugin_glob_all(${PLUGIN_NAME})

# Find dependencies
plugin_add_event_model(${PLUGIN_NAME})

# Find PandoraPFA (optional, for algorithm registration)
find_package(PandoraPFA CONFIG QUIET)

# Add libraries (same as target_include_directories but for both plugin and
# library)
if(PandoraPFA_FOUND)
  plugin_link_libraries(${PLUGIN_NAME} PandoraPFA::PandoraSDK PandoraPFA::PandoraPFA DD4hep::DDRec)
else()
  plugin_link_libraries(${PLUGIN_NAME} PandoraPFA::PandoraSDK DD4hep::DDRec)
endif()

# Install default Pandora settings XML alongside the library
install(FILES data/PandoraPFASettings.xml
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}/pandora
)
