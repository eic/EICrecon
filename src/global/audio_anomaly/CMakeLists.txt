# Audio Anomaly Detection Plugin

# Create the plugin
plugin_add(audio_anomaly)

# Sources - main plugin and test processor
plugin_glob_all(audio_anomaly_plugin_sources *.cc)
plugin_sources(audio_anomaly ${audio_anomaly_plugin_sources})