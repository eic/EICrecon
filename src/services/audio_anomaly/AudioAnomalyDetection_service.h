#pragma once

#include <JANA/JApplicationFwd.h>
#include <JANA/JServiceFwd.h>
#include <spdlog/logger.h>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <atomic>
#include <mutex>

/**
 * AudioAnomalyDetection_service provides audio feedback for anomaly detection
 * in subatomic physics data analysis. Each detector subsystem is mapped to a
 * different frequency band, with loudness proportional to anomaly levels.
 */
class AudioAnomalyDetection_service : public JService {
public:
  AudioAnomalyDetection_service(JApplication* app);
  virtual ~AudioAnomalyDetection_service();

  /**
     * Report anomaly level for a specific detector subsystem
     * @param detector_name Name of the detector subsystem (e.g., "BEMC", "BHCAL", etc.)
     * @param anomaly_level Normalized anomaly level (0.0 = no anomaly, 1.0 = max anomaly)
     */
  virtual void reportAnomaly(const std::string& detector_name, double anomaly_level);

  /**
     * Start the audio output stream
     */
  virtual void startAudioStream();

  /**
     * Stop the audio output stream
     */
  virtual void stopAudioStream();

  /**
     * Check if audio stream is currently active
     */
  virtual bool isStreamActive() const;

private:
  AudioAnomalyDetection_service() = default;
  void acquire_services(JServiceLocator*) override;

  // Audio generation and output
  void audioGenerationLoop();
  void initializeAudio();
  void cleanupAudio();

  // Frequency mapping for detectors
  void initializeFrequencyMapping();
  double getFrequencyForDetector(const std::string& detector_name) const;

  // Audio synthesis
  void generateAudioFrame(std::vector<float>& buffer);
  void mixSineWave(std::vector<float>& buffer, double frequency, double amplitude, double& phase);

  JApplication* m_app;
  std::shared_ptr<spdlog::logger> m_log;

  // Audio parameters
  std::string m_audio_device;
  int m_sample_rate;
  int m_buffer_size;
  int m_channels;

  // Detector frequency mapping
  std::map<std::string, double> m_detector_frequencies;
  std::map<std::string, double> m_detector_phases;

  // Current anomaly levels (thread-safe)
  mutable std::mutex m_anomaly_mutex;
  std::map<std::string, double> m_current_anomalies;

  // Audio thread management
  std::atomic<bool> m_running;
  std::thread m_audio_thread;

  // ALSA handles (forward declared to avoid including ALSA headers in header)
  void* m_pcm_handle;
};
