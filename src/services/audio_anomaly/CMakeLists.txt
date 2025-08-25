# Audio anomaly detection service

# Find ALSA library
find_package(PkgConfig REQUIRED)
pkg_check_modules(ALSA REQUIRED alsa)

# Create the plugin
plugin_add(audio_anomaly WITH_LIBRARY)

# Audio anomaly detection sources
plugin_glob_all(audio_anomaly_sources AudioAnomalyDetection_service.cc)
plugin_sources(audio_anomaly ${audio_anomaly_sources})

# Add ALSA include directories and libraries
plugin_include_directories(audio_anomaly SYSTEM PUBLIC ${ALSA_INCLUDE_DIRS})
plugin_link_libraries(audio_anomaly ${ALSA_LIBRARIES})

# Installation
plugin_glob_all(audio_anomaly_headers *.h)
plugin_install_headers(audio_anomaly ${audio_anomaly_headers})