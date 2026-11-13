// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzE4D.h>
#include <Math/GenVector/VectorUtil.h>
#include <Math/Vector4Dfwd.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <format>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <vector>

#include "InclusiveKinematicsTruthDressed.h"
#include "TruthLeptonLine.h"
#include "algorithms/reco/InclusiveKinematicsTruthDressedConfig.h"

using ROOT::Math::PxPyPzEVector;

namespace eicrecon {

void InclusiveKinematicsTruthDressed::init() {
  if (m_cfg.coneDeltaR < 0) {
    error("Dressing cone size coneDeltaR = {} must not be negative", m_cfg.coneDeltaR);
    throw std::runtime_error(
        std::format("Dressing cone size coneDeltaR = {} must not be negative", m_cfg.coneDeltaR));
  }
}

void InclusiveKinematicsTruthDressed::process(
    const InclusiveKinematicsTruthDressed::Input& input,
    const InclusiveKinematicsTruthDressed::Output& output) const {

  const auto [mcparts]                            = input;
  auto [kinematics, scattered, scat_constituents] = output;
  scat_constituents->setSubsetCollection();

  // Get lepton line of the signal interaction
  const auto truth = find_lepton_line_truth(*mcparts);
  if (!truth.has_value()) {
    debug("No beam electron with a final-state scattered lepton found");
    return;
  }

  // Get incoming electron beam
  static const auto ei_mass = m_particleSvc.particle(11).mass;
  const PxPyPzEVector ei    = four_momentum(truth->beam_lepton(), ei_mass);

  // Get incoming hadron beam
  if (!truth->beam_hadron.has_value()) {
    debug("No beam hadron found");
    return;
  }
  const auto& pi_particle = *truth->beam_hadron;
  const auto pi_beam      = beam_per_nucleon(
      pi_particle,
      is_ion(pi_particle.getPDG()) ? 0. : m_particleSvc.particle(pi_particle.getPDG()).mass);

  // Get bare scattered lepton
  const auto& ef_particle     = truth->scattered_lepton();
  const auto ef_mass          = m_particleSvc.particle(std::abs(ef_particle.getPDG())).mass;
  const PxPyPzEVector ef_bare = four_momentum(ef_particle, ef_mass);

  // Select photons radiated from the scattered lepton (only charged leptons radiate): by
  // parentage, when the record has photons emitted after the hard scattering (e.g. DJANGOH
  // 4.6.21, Pythia8 with QED showers); otherwise inside the dressing cone around the bare lepton
  // (e.g. DJANGOH 4.6.10, which attaches all photons to the beam lepton)
  std::vector<edm4hep::MCParticle> fsr_photons;
  if (ef_particle.getCharge() != 0) {
    const bool use_parentage =
        std::any_of(truth->photons.begin(), truth->photons.end(),
                    [&truth](const auto& photon) { return truth->is_fsr(photon); });
    for (const auto& photon : truth->photons) {
      const bool selected = use_parentage
                                ? truth->is_fsr(photon)
                                : ROOT::Math::VectorUtil::DeltaR(four_momentum(photon.particle),
                                                                 ef_bare) < m_cfg.coneDeltaR;
      if (selected) {
        fsr_photons.push_back(photon.particle);
      }
    }
    trace("{} of {} lepton-line photons added by {}", fsr_photons.size(), truth->photons.size(),
          use_parentage ? "parentage" : "cone");
  }

  // Dressed scattered lepton
  PxPyPzEVector ef = ef_bare;
  scat_constituents->push_back(ef_particle);
  for (const auto& photon : fsr_photons) {
    ef += four_momentum(photon);
    scat_constituents->push_back(photon);
  }

  auto scat = scattered->create();
  scat.setType(ef_particle.getGeneratorStatus());
  scat.setEnergy(ef.E());
  scat.setMomentum(
      {static_cast<float>(ef.Px()), static_cast<float>(ef.Py()), static_cast<float>(ef.Pz())});
  const auto& vertex = ef_particle.getVertex();
  scat.setReferencePoint(
      {static_cast<float>(vertex.x), static_cast<float>(vertex.y), static_cast<float>(vertex.z)});
  scat.setCharge(ef_particle.getCharge());
  scat.setMass(ef.M());
  scat.setGoodnessOfPID(1);
  scat.setPDG(ef_particle.getPDG());

  // DIS kinematics calculations
  const auto q   = ei - ef;
  const auto dis = dis_variables(ei, pi_beam.momentum, pi_beam.mass, q);
  auto kin       = kinematics->create(dis.x, dis.Q2, dis.W, dis.y, dis.nu);
  kin.setScat(scat);

  debug("x,Q2,W,y,nu = {},{},{},{},{}", kin.getX(), kin.getQ2(), kin.getW(), kin.getY(),
        kin.getNu());
}

} // namespace eicrecon
