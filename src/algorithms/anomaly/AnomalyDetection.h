#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <string>
#include <string_view>

#include "AnomalyDetectionConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

/**
 * Algorithm to compute anomaly levels by comparing truth and reconstructed particles
 */
using AnomalyDetectionAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::MCParticleCollection, edm4eic::ReconstructedParticleCollection>,
    algorithms::Output<>>;

class AnomalyDetection : public AnomalyDetectionAlgorithm,
                         public WithPodConfig<AnomalyDetectionConfig> {

public:
  AnomalyDetection(std::string_view name)
      : AnomalyDetectionAlgorithm{name,
                                  {"MCParticles", "ReconstructedParticles"},
                                  {},
                                  "Compute anomaly levels between truth and reconstructed data"} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  /**
     * Compute energy-based anomaly for a detector subsystem
     */
  double computeEnergyAnomaly(const edm4hep::MCParticleCollection& mc_particles,
                              const edm4eic::ReconstructedParticleCollection& reco_particles,
                              const std::string& detector_name) const;

  /**
     * Compute momentum-based anomaly
     */
  double computeMomentumAnomaly(const edm4hep::MCParticleCollection& mc_particles,
                                const edm4eic::ReconstructedParticleCollection& reco_particles,
                                const std::string& detector_name) const;

  /**
     * Normalize anomaly level to [0,1] range
     */
  double normalizeAnomaly(double raw_anomaly) const;
};

} // namespace eicrecon
