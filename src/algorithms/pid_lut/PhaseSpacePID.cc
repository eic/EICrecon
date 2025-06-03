// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Nathan Brei, Dmitry Kalinkin, Simon Gardner

#include <algorithms/service.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <algorithms/geo.h>
#include <fmt/core.h>
#include <cmath>
#include <gsl/pointers>
#include <stdexcept>

#include "algorithms/interfaces/ParticleSvc.h"
#include "algorithms/pid_lut/PhaseSpacePID.h"
#include "algorithms/pid_lut/PhaseSpacePIDConfig.h"

namespace eicrecon {

void PhaseSpacePID::init() {  
  
  auto detector = algorithms::GeoSvc::instance().detector();

  try {
    m_system = detector->constant<int32_t>(m_cfg.system);
  } catch (const std::exception& e) {
    error("Failed to get {} from the detector: {}", m_cfg.system, e.what());
    throw std::runtime_error("Failed to get requested ID from the detector");
  }

  m_direction = edm4hep::Vector3f{m_cfg.direction[0], m_cfg.direction[1], m_cfg.direction[2]};
  
  
  auto& particleSvc = algorithms::ParticleSvc::instance();
  m_mass   = particleSvc.particle(m_cfg.pdg_value).mass;
  m_charge = particleSvc.particle(m_cfg.pdg_value).charge;

}

void PhaseSpacePID::process(const Input& input, const Output& output) const {
  const auto [recoparts_in, partassocs_in]          = input;
  auto [recoparts_out, partassocs_out, partids_out] = output;

  for (const auto& recopart_without_pid : *recoparts_in) {
    auto recopart = recopart_without_pid.clone();

    // Find MCParticle from associations and propagate the relevant ones further
    auto best_assoc = edm4eic::MCRecoParticleAssociation::makeEmpty();
    for (auto assoc_in : *partassocs_in) {
      if (assoc_in.getRec() == recopart_without_pid) {
        if ((not best_assoc.isAvailable()) || (best_assoc.getWeight() < assoc_in.getWeight())) {
          best_assoc = assoc_in;
        }
        auto assoc_out = assoc_in.clone();
        assoc_out.setRec(recopart);
        partassocs_out->push_back(assoc_out);
      }
    }
    if (not best_assoc.isAvailable()) {
      recoparts_out->push_back(recopart);
      continue;
    }


    edm4hep::MCParticle mcpart = best_assoc.getSim();

    // Check if particle is within the phase space
    edm4hep::Vector3f momentum = recopart.getMomentum();
    double angle = edm4hep::utils::angleBetween(momentum, m_direction);
    if (angle < m_cfg.opening_angle) {

      for( auto& test_pdg: m_pdg_potential_values){
        bool match = (test_pdg==m_cfg.pdg_value);


        recopart.addToParticleIDs(
            partids_out->create(m_system,                 // std::int32_t type
                                test_pdg,                 // std::int32_t PDG
                                0,                        // std::int32_t algorithmType
                                static_cast<float>(match) // float likelihood
                                ));

        if(match) {
          recopart.setParticleIDUsed((*partids_out)[partids_out->size() - 1]);
        }
      }

      recopart.setPDG(m_cfg.pdg_value);
      recopart.setMass(m_mass);

    }
    recoparts_out->push_back(recopart);
  }
}

} // namespace eicrecon
