// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Sebouh Paul, Baptiste Fraisse

#pragma once
#include <DD4hep/Detector.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <spdlog/logger.h>
#include <gsl/pointers>
#include <memory>
#include <string>      // for basic_string
#include <string_view> // for string_view

#include "algorithms/interfaces/WithPodConfig.h"
#include "services/particle/ParticleSvc.h"
#include "algorithms/reco/FarForwardLambdaReconstructionConfig.h"

namespace eicrecon {

using FarForwardLambdaReconstructionAlgorithm = algorithms::Algorithm<
    algorithms::Input<const edm4eic::ReconstructedParticleCollection,
                      const edm4eic::ReconstructedParticleCollection,
                      const edm4eic::ReconstructedParticleCollection,
                      const edm4eic::ReconstructedParticleCollection>,
    /*output collections contain the lambda candidates and their decay products in the CM frame*/
    algorithms::Output<edm4eic::ReconstructedParticleCollection,
                       edm4eic::ReconstructedParticleCollection>>;
/**
 * Reconstruct far-forward Lambda candidates from neutral reconstructed particles.
 *
 * The reconstruction proceeds in five stages:
 * 1. collect photon and neutron candidates from the configured far-forward
 *   neutral collections and split them into detector categories;
 * 2. build pi0 candidates from all photon pairs passing the configured pi0
 *   invariant-mass window;
 * 3. combine accepted pi0 candidates with neutron candidates to form Lambda
 *   candidates passing the configured Lambda invariant-mass window;
 * 4. rank the surviving candidates according to detector-preference and
 *   mass-compatibility criteria;
 * 5. run the final Lambda reconstruction on the best candidate and store the
 *   resulting Lambda and decay products.
 *
 * The current ranking gives priority to candidates containing a ZDC neutron,
 * then to candidates containing more ZDC photons, followed by the combined
 * pi0/Lambda mass residual score and forward kinematics.
 */
class FarForwardLambdaReconstruction : public FarForwardLambdaReconstructionAlgorithm,
                                       public WithPodConfig<FarForwardLambdaReconstructionConfig> {
public:
  FarForwardLambdaReconstruction(std::string_view name)
      : FarForwardLambdaReconstructionAlgorithm{
            name,

            {"inputNeutralsHcal", "inputNeutralsB0", "inputNeutralsEcalEndCapP",
             "inputNeutralsLFHCAL"},

            {"outputLambdas", "outputLambdaDecayProductsCM"},

            "Reconstructs lambda candidates and their decay products (in the CM frame) from the "
            "reconstructed neutrons and photons"} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  std::shared_ptr<spdlog::logger> m_log;
  const algorithms::ParticleSvc& m_particleSvc = algorithms::ParticleSvc::instance();
  const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};
  double m_zMax{0};

  bool reconstruct_from_triplet(const edm4eic::ReconstructedParticle& n_in,
                                const edm4eic::ReconstructedParticle& g1_in,
                                const edm4eic::ReconstructedParticle& g2_in,
                                edm4eic::ReconstructedParticleCollection* out_lambdas,
                                edm4eic::ReconstructedParticleCollection* out_decay_products) const;
};
} // namespace eicrecon
