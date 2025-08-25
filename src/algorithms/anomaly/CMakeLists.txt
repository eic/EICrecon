# Anomaly Detection Algorithm

# Create the plugin
plugin_add(anomaly)

# Sources
plugin_glob_all(anomaly_sources *.cc)
plugin_sources(anomaly ${anomaly_sources})

# Headers
plugin_glob_all(anomaly_headers *.h)
plugin_install_headers(anomaly ${anomaly_headers})
