# Anomaly Detection Factory

# Create the plugin
plugin_add(anomaly_factory)

# Sources
plugin_glob_all(anomaly_factory_sources *.cc)
plugin_sources(anomaly_factory ${anomaly_factory_sources})

# Headers  
plugin_glob_all(anomaly_factory_headers *.h)
plugin_install_headers(anomaly_factory ${anomaly_factory_headers})