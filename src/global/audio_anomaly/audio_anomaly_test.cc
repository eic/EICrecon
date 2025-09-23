// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 EIC Reconstruction Contributors

#include <JANA/JApplication.h>
#include <JANA/JEventProcessor.h>
#include <services/audio_anomaly/AudioAnomalyDetection_service.h>
#include <random>
#include <iostream>
#include <chrono>

/**
 * Test processor that demonstrates the Audio Anomaly Detection Service
 * by generating simulated anomaly data
 */
class AudioAnomalyTest_processor : public JEventProcessor {
public:
  AudioAnomalyTest_processor() { SetTypeName(NAME_OF_THIS); }

  void Init() override {
    auto app = GetApplication();

    // Get the audio anomaly service
    m_audio_service = app->GetService<AudioAnomalyDetection_service>();
    if (!m_audio_service) {
      std::cerr << "AudioAnomalyDetection_service not available" << std::endl;
      return;
    }

    // Start the audio stream
    m_audio_service->startAudioStream();

    // Initialize random number generator for simulated anomalies
    m_random_engine.seed(std::chrono::system_clock::now().time_since_epoch().count());

    std::cout << "AudioAnomalyTest_processor initialized" << std::endl;
  }

  void Process(const std::shared_ptr<const JEvent>& event) override {
    if (!m_audio_service)
      return;

    // Simulate anomaly detection for different detectors
    std::vector<std::string> detectors = {"BEMC", "BHCAL", "EEMC", "EHCAL", "FEMC",   "FHCAL",
                                          "BTRK", "ECTRK", "BVTX", "DRICH", "PFRICH", "DIRC",
                                          "BTOF", "ECTOF", "ZDC",  "B0TRK", "B0ECAL"};

    for (const auto& detector : detectors) {
      // Generate random anomaly level (with bias toward low values)
      double anomaly_level = generateRandomAnomaly();

      // Report to audio service
      m_audio_service->reportAnomaly(detector, anomaly_level);
    }

    // Log progress every 100 events
    static int event_count = 0;
    event_count++;
    if (event_count % 100 == 0) {
      std::cout << "Processed " << event_count << " events" << std::endl;
    }
  }

  void Finish() override {
    if (m_audio_service) {
      m_audio_service->stopAudioStream();
    }
    std::cout << "AudioAnomalyTest_processor finished" << std::endl;
  }

private:
  /**
     * Generate a realistic anomaly level
     * Most values are low (normal operation), with occasional spikes
     */
  double generateRandomAnomaly() {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double base_value = dist(m_random_engine);

    // 90% of the time, return very low anomaly (normal operation)
    if (base_value < 0.9) {
      return base_value * 0.1; // 0.0 to 0.1
    }
    // 9% of the time, return moderate anomaly
    else if (base_value < 0.99) {
      return 0.1 + (base_value - 0.9) * 3.0; // 0.1 to 0.4
    }
    // 1% of the time, return high anomaly
    else {
      return 0.4 + (base_value - 0.99) * 60.0; // 0.4 to 1.0
    }
  }

  AudioAnomalyDetection_service* m_audio_service = nullptr;
  std::default_random_engine m_random_engine;
};

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  // Register the audio anomaly detection service
  app->ProvideService(std::make_shared<AudioAnomalyDetection_service>(app));

  // Add the test processor
  app->Add(new AudioAnomalyTest_processor());
}
}
