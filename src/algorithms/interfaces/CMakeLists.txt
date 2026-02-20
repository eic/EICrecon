set(PLUGIN_NAME "algorithms_interfaces")

# Function creates ${PLUGIN_NAME}_plugin and ${PLUGIN_NAME}_library targets
# Setting default includes, libraries and installation paths
plugin_add(${PLUGIN_NAME} WITH_SHARED_LIBRARY)

# Add sources explicitly
plugin_sources(${PLUGIN_NAME} ActsSvc.cc)

# Find dependencies
plugin_add_acts(${PLUGIN_NAME})

# Add tracking library dependency for ActsDD4hepDetector classes
plugin_link_libraries(${PLUGIN_NAME} algorithms_tracking_library)
