# Automatically set plugin name the same as the directory name
eicrecon_set_plugin_name()

# Function creates ${PLUGIN_NAME}_plugin and ${PLUGIN_NAME}_library targets
# Setting default includes, libraries and installation paths
eicrecon_add(${PLUGIN_NAME})

# The macro grabs sources as *.cc *.cpp *.c and headers as *.h *.hh *.hpp
# Then correctly sets sources for ${_name}_plugin and ${_name}_library targets
# Adds headers to the correct installation directory
eicrecon_glob_all(${PLUGIN_NAME})

# Find dependencies
eicrecon_add_event_model(${PLUGIN_NAME})
