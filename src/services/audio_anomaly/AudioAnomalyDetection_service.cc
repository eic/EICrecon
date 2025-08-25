// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 EIC Reconstruction Contributors

#include "AudioAnomalyDetection_service.h"
#include <JANA/JApplication.h>
#include <JANA/Services/JServiceLocator.h>
#include <services/log/Log_service.h>
#include <alsa/asoundlib.h>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <chrono>

AudioAnomalyDetection_service::AudioAnomalyDetection_service(JApplication* app) 
    : m_app(app), m_running(false), m_pcm_handle(nullptr) {
    
    // Set default audio parameters
    m_audio_device = "default";
    m_sample_rate = 44100;
    m_buffer_size = 1024;
    m_channels = 1;
}

AudioAnomalyDetection_service::~AudioAnomalyDetection_service() {
    stopAudioStream();
    cleanupAudio();
}

void AudioAnomalyDetection_service::acquire_services(JServiceLocator* srv_locator) {
    auto log_service = srv_locator->get<Log_service>();
    m_log = log_service->logger("AudioAnomalyDetection");
    
    // Get audio device parameter from configuration
    auto pm = m_app->GetJParameterManager();
    pm->SetDefaultParameter("audio_anomaly:device", m_audio_device, 
                           "Audio device for anomaly detection output");
    pm->SetDefaultParameter("audio_anomaly:sample_rate", m_sample_rate,
                           "Sample rate for audio output (Hz)");
    pm->SetDefaultParameter("audio_anomaly:buffer_size", m_buffer_size,
                           "Audio buffer size in samples");
    
    m_log->info("AudioAnomalyDetection_service initialized with device: {}", m_audio_device);
    
    initializeFrequencyMapping();
    initializeAudio();
}

void AudioAnomalyDetection_service::initializeFrequencyMapping() {
    // Map detector subsystems to distinct frequency bands
    // Using frequencies between 200 Hz and 2000 Hz for good audibility
    m_detector_frequencies["BEMC"]    = 300.0;   // Barrel Electromagnetic Calorimeter
    m_detector_frequencies["BHCAL"]   = 400.0;   // Barrel Hadronic Calorimeter
    m_detector_frequencies["EEMC"]    = 500.0;   // Endcap Electromagnetic Calorimeter
    m_detector_frequencies["EHCAL"]   = 600.0;   // Endcap Hadronic Calorimeter
    m_detector_frequencies["FEMC"]    = 700.0;   // Far Forward Electromagnetic Calorimeter
    m_detector_frequencies["FHCAL"]   = 800.0;   // Far Forward Hadronic Calorimeter
    m_detector_frequencies["BTRK"]    = 900.0;   // Barrel Tracker
    m_detector_frequencies["ECTRK"]   = 1000.0;  // Endcap Tracker
    m_detector_frequencies["BVTX"]    = 1100.0;  // Barrel Vertex Detector
    m_detector_frequencies["DRICH"]   = 1200.0;  // Dual Ring Imaging Cherenkov
    m_detector_frequencies["PFRICH"]  = 1300.0;  // Proximity Focusing RICH
    m_detector_frequencies["DIRC"]    = 1400.0;  // Detection of Internally Reflected Cherenkov
    m_detector_frequencies["BTOF"]    = 1500.0;  // Barrel Time of Flight
    m_detector_frequencies["ECTOF"]   = 1600.0;  // Endcap Time of Flight
    m_detector_frequencies["ZDC"]     = 1700.0;  // Zero Degree Calorimeter
    m_detector_frequencies["B0TRK"]   = 1800.0;  // B0 Tracker
    m_detector_frequencies["B0ECAL"]  = 1900.0;  // B0 Electromagnetic Calorimeter
    
    // Initialize phase accumulator for each detector
    for (const auto& pair : m_detector_frequencies) {
        m_detector_phases[pair.first] = 0.0;
    }
    
    m_log->info("Initialized frequency mapping for {} detectors", m_detector_frequencies.size());
}

void AudioAnomalyDetection_service::initializeAudio() {
    snd_pcm_t* handle;
    snd_pcm_hw_params_t* params;
    
    // Try to open PCM device for playback
    int rc = snd_pcm_open(&handle, m_audio_device.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0) {
        m_log->warn("Unable to open PCM device {}: {} - running in silent mode", 
                   m_audio_device, snd_strerror(rc));
        // Set handle to nullptr to indicate silent mode
        m_pcm_handle = nullptr;
        return;
    }
    
    m_pcm_handle = static_cast<void*>(handle);
    
    // Allocate parameters object
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(handle, params);
    
    // Set parameters
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_FLOAT);
    snd_pcm_hw_params_set_channels(handle, params, m_channels);
    
    unsigned int sample_rate = m_sample_rate;
    snd_pcm_hw_params_set_rate_near(handle, params, &sample_rate, nullptr);
    
    snd_pcm_uframes_t frames = m_buffer_size;
    snd_pcm_hw_params_set_period_size_near(handle, params, &frames, nullptr);
    
    // Apply parameters
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        m_log->error("Unable to set hw parameters: {}", snd_strerror(rc));
        snd_pcm_close(handle);
        m_pcm_handle = nullptr;
        return;
    }
    
    m_log->info("Audio initialized: {} Hz, {} channels, {} samples buffer", 
                sample_rate, m_channels, frames);
}

void AudioAnomalyDetection_service::cleanupAudio() {
    if (m_pcm_handle) {
        snd_pcm_t* handle = static_cast<snd_pcm_t*>(m_pcm_handle);
        snd_pcm_close(handle);
        m_pcm_handle = nullptr;
    }
}

void AudioAnomalyDetection_service::reportAnomaly(const std::string& detector_name, double anomaly_level) {
    // Clamp anomaly level to valid range
    anomaly_level = std::clamp(anomaly_level, 0.0, 1.0);
    
    {
        std::lock_guard<std::mutex> lock(m_anomaly_mutex);
        m_current_anomalies[detector_name] = anomaly_level;
    }
    
    m_log->trace("Anomaly reported for {}: {:.3f}", detector_name, anomaly_level);
}

void AudioAnomalyDetection_service::startAudioStream() {
    if (m_running.load()) {
        return;
    }
    
    m_running.store(true);
    m_audio_thread = std::thread(&AudioAnomalyDetection_service::audioGenerationLoop, this);
    
    if (m_pcm_handle) {
        m_log->info("Audio stream started with device output");
    } else {
        m_log->info("Audio stream started in silent mode (no audio device available)");
    }
}

void AudioAnomalyDetection_service::stopAudioStream() {
    if (!m_running.load()) {
        return;
    }
    
    m_running.store(false);
    if (m_audio_thread.joinable()) {
        m_audio_thread.join();
    }
    m_log->info("Audio stream stopped");
}

bool AudioAnomalyDetection_service::isStreamActive() const {
    return m_running.load();
}

void AudioAnomalyDetection_service::audioGenerationLoop() {
    std::vector<float> buffer(m_buffer_size * m_channels);
    
    while (m_running.load()) {
        try {
            // Generate audio frame
            generateAudioFrame(buffer);
            
            if (m_pcm_handle) {
                // Write to audio device if available
                snd_pcm_t* handle = static_cast<snd_pcm_t*>(m_pcm_handle);
                int rc = snd_pcm_writei(handle, buffer.data(), m_buffer_size);
                if (rc == -EPIPE) {
                    // Buffer underrun - recover
                    m_log->warn("Audio buffer underrun, attempting recovery");
                    snd_pcm_prepare(handle);
                } else if (rc < 0) {
                    m_log->error("Audio write error: {}", snd_strerror(rc));
                    break;
                }
            } else {
                // Silent mode - just simulate timing
                std::this_thread::sleep_for(
                    std::chrono::microseconds(m_buffer_size * 1000000 / m_sample_rate));
            }
        } catch (const std::exception& e) {
            m_log->error("Audio generation error: {}", e.what());
            break;
        }
    }
}

void AudioAnomalyDetection_service::generateAudioFrame(std::vector<float>& buffer) {
    // Clear buffer
    std::fill(buffer.begin(), buffer.end(), 0.0f);
    
    // Mix sine waves for each detector with current anomaly levels
    std::lock_guard<std::mutex> lock(m_anomaly_mutex);
    
    for (const auto& detector_pair : m_current_anomalies) {
        const std::string& detector_name = detector_pair.first;
        double anomaly_level = detector_pair.second;
        
        if (anomaly_level > 0.0) {
            auto freq_it = m_detector_frequencies.find(detector_name);
            if (freq_it != m_detector_frequencies.end()) {
                double frequency = freq_it->second;
                double amplitude = anomaly_level * 0.1; // Scale to prevent clipping
                
                mixSineWave(buffer, frequency, amplitude, m_detector_phases[detector_name]);
            }
        }
    }
}

void AudioAnomalyDetection_service::mixSineWave(std::vector<float>& buffer, double frequency, 
                                               double amplitude, double& phase) {
    const double phase_increment = 2.0 * M_PI * frequency / m_sample_rate;
    
    for (size_t i = 0; i < buffer.size(); i += m_channels) {
        float sample = static_cast<float>(amplitude * std::sin(phase));
        
        // Mix into all channels
        for (int ch = 0; ch < m_channels; ++ch) {
            buffer[i + ch] += sample;
        }
        
        phase += phase_increment;
        if (phase >= 2.0 * M_PI) {
            phase -= 2.0 * M_PI;
        }
    }
}