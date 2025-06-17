// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Sebouh Paul

#pragma once
#include <DD4hep/Detector.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <spdlog/logger.h>
#include <gsl/pointers>
#include <memory>
#include <string>      // for basic_string
#include <string_view> // for string_view
#include <vector>

#include "algorithms/interfaces/ParticleSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/reco/FarForwardNeutralsReconstructionConfig.h"

namespace eicrecon {

using FarForwardNeutralsReconstructionAlgorithm =
    algorithms::Algorithm<algorithms::Input<const edm4eic::ClusterCollection>,
                          algorithms::Output<edm4eic::ReconstructedParticleCollection>>;
class FarForwardNeutralsReconstruction
    : public FarForwardNeutralsReconstructionAlgorithm,
      public WithPodConfig<FarForwardNeutralsReconstructionConfig> {
public:
  FarForwardNeutralsReconstruction(std::string_view name)
      : FarForwardNeutralsReconstructionAlgorithm{name,
                                                  {"inputClustersHcal"},
                                                  {"outputNeutrals"},
                                                  "Merges all HCAL clusters in a collection into a "
                                                  "neutron candidate and photon candidates "} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  static double calc_corr(double Etot, const std::vector<double>&);
  bool isGamma(const edm4eic::Cluster& cluster) const;

  std::shared_ptr<spdlog::logger> m_log;
  const algorithms::ParticleSvc& m_particleSvc = algorithms::ParticleSvc::instance();
  const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};
  double m_gammaZMax{0};
};
} // namespace eicrecon
