// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Dmitry Romanov

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <string>      // for basic_string
#include <string_view> // for string_view

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/reco/ChargedLambdaReconstructionConfig.h"
#include "services/particle/ParticleSvc.h"

namespace eicrecon {

using ChargedLambdaReconstructionAlgorithm =
    algorithms::Algorithm<algorithms::Input<const edm4eic::ReconstructedParticleCollection,
                                            const edm4eic::ReconstructedParticleCollection,
                                            const edm4eic::ReconstructedParticleCollection>,
                          algorithms::Output<edm4eic::ReconstructedParticleCollection>>;

/**
 * Reconstruct Lambda candidates in the charged decay channel Lambda -> p pi-.
 *
 * Lambdas produced in ep collisions are strongly forward boosted and typically decay
 * meters downstream of the interaction point, so the daughters land in the far-forward
 * detectors: the proton in the Roman Pots, the off-momentum tracker or the B0 tracker,
 * the pi- in the B0 tracker (occasionally in the central detector). The algorithm
 * therefore combines three input collections:
 *
 * 1. charged particles from tracking (central + B0): positive candidates enter the
 *    proton pool, negative candidates enter the pion pool;
 * 2. Roman-Pot candidates: all enter the proton pool (transfer-matrix reconstruction
 *    under the proton hypothesis);
 * 3. off-momentum candidates: all enter the proton pool, same reasoning.
 *
 * Every (p, pi-) pairing is evaluated with a classic V0 cut chain computed from
 * momenta and reference points only:
 *   - opening angle between the daughters and polar angle of the summed momentum;
 *   - distance of closest approach between the two straight track lines, and the z of
 *     the closest-approach midpoint (a decay-vertex proxy);
 *   - pointing of the summed momentum along the origin-to-vertex-proxy direction
 *     (disabled by default, see ChargedLambdaReconstructionConfig::pointingMax);
 *   - momentum asymmetry (the proton carries most of the Lambda momentum);
 *   - invariant mass window under the (m_p, m_pi) hypothesis.
 *
 * All pairings that pass are stored as Lambda candidates with the daughters attached
 * via the particles relation; no best-candidate choice is made here, so analyses keep
 * the full candidate set and the mass sidebands within the configured window.
 */
class ChargedLambdaReconstruction : public ChargedLambdaReconstructionAlgorithm,
                                    public WithPodConfig<ChargedLambdaReconstructionConfig> {
public:
  ChargedLambdaReconstruction(std::string_view name)
      : ChargedLambdaReconstructionAlgorithm{
            name,
            {"inputChargedParticles", "inputRomanPotParticles", "inputOffMomentumParticles"},
            {"outputLambdas"},
            "Reconstructs Lambda -> p pi- candidates from charged and far-forward "
            "particles with V0 topology and kinematics cuts"} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const algorithms::ParticleSvc& m_particleSvc = algorithms::ParticleSvc::instance();
  double m_proton_mass{0};
  double m_pion_mass{0};
  double m_lambda_mass{0};
};

} // namespace eicrecon
