// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 EIC Reconstruction Contributors

#include "AnomalyDetection.h"
#include <algorithms/service.h>
#include <edm4hep/utils/vector_utils.h>
#include <cmath>
#include <numeric>

namespace eicrecon {

void AnomalyDetection::init() {
    // Get the audio service for reporting anomalies
    auto& serviceSvc = algorithms::ServiceSvc::instance();
    // The service will be accessed during processing
}

void AnomalyDetection::process(const Input& input, const Output& output) const {
    
    const auto [mc_particles, reco_particles] = input;
    
    // Try to get application from static access
    // Note: In a real implementation, this would be passed through the service infrastructure
    static int event_count = 0;
    event_count++;
    
    // Only process every N events to avoid overwhelming the audio system
    if (event_count % m_cfg.update_frequency != 0) {
        return;
    }
    
    debug("Processing anomaly detection for event {}", event_count);
    
    // Compute anomalies for each detector subsystem
    for (const std::string& detector : m_cfg.detector_systems) {
        
        // Compute energy-based anomaly
        double energy_anomaly = computeEnergyAnomaly(*mc_particles, *reco_particles, detector);
        
        // Compute momentum-based anomaly
        double momentum_anomaly = computeMomentumAnomaly(*mc_particles, *reco_particles, detector);
        
        // Combined anomaly (weighted average)
        double combined_anomaly = (energy_anomaly + momentum_anomaly) / 2.0;
        
        // Normalize to [0,1] range
        double normalized_anomaly = normalizeAnomaly(combined_anomaly);
        
        trace("Detector {}: energy_anomaly={:.3f}, momentum_anomaly={:.3f}, normalized={:.3f}", 
              detector, energy_anomaly, momentum_anomaly, normalized_anomaly);
        
        // Report to audio service (would need proper service access in real implementation)
        // For now, just log the result
        if (normalized_anomaly > 0.1) {
            info("Anomaly detected in {}: level={:.3f}", detector, normalized_anomaly);
        }
    }
}

double AnomalyDetection::computeEnergyAnomaly(const edm4hep::MCParticleCollection& mc_particles,
                                            const edm4eic::ReconstructedParticleCollection& reco_particles,
                                            const std::string& detector_name) const {
    
    // Simple energy-based anomaly: compare total energy in truth vs reconstructed
    double mc_total_energy = 0.0;
    double reco_total_energy = 0.0;
    
    // Sum energy from MC particles
    for (const auto& mc_particle : mc_particles) {
        double energy = mc_particle.getEnergy();
        if (energy > m_cfg.energy_threshold) {
            mc_total_energy += energy;
        }
    }
    
    // Sum energy from reconstructed particles
    for (const auto& reco_particle : reco_particles) {
        double energy = reco_particle.getEnergy();
        if (energy > m_cfg.energy_threshold) {
            reco_total_energy += energy;
        }
    }
    
    // Calculate relative difference
    if (mc_total_energy > 0.0) {
        return std::abs(reco_total_energy - mc_total_energy) / mc_total_energy;
    }
    
    return 0.0;
}

double AnomalyDetection::computeMomentumAnomaly(const edm4hep::MCParticleCollection& mc_particles,
                                              const edm4eic::ReconstructedParticleCollection& reco_particles,
                                              const std::string& detector_name) const {
    
    // Momentum-based anomaly: compare momentum magnitude distributions
    double mc_total_momentum = 0.0;
    double reco_total_momentum = 0.0;
    
    // Sum momentum from MC particles
    for (const auto& mc_particle : mc_particles) {
        auto momentum = mc_particle.getMomentum();
        double p_mag = edm4hep::utils::magnitude(momentum);
        if (p_mag > m_cfg.momentum_threshold) {
            mc_total_momentum += p_mag;
        }
    }
    
    // Sum momentum from reconstructed particles
    for (const auto& reco_particle : reco_particles) {
        auto momentum = reco_particle.getMomentum();
        double p_mag = edm4hep::utils::magnitude(momentum);
        if (p_mag > m_cfg.momentum_threshold) {
            reco_total_momentum += p_mag;
        }
    }
    
    // Calculate relative difference
    if (mc_total_momentum > 0.0) {
        return std::abs(reco_total_momentum - mc_total_momentum) / mc_total_momentum;
    }
    
    return 0.0;
}

double AnomalyDetection::normalizeAnomaly(double raw_anomaly) const {
    // Clamp to maximum value and normalize to [0,1]
    double clamped = std::min(raw_anomaly, m_cfg.max_anomaly_value);
    return clamped / m_cfg.max_anomaly_value;
}

} // namespace eicrecon