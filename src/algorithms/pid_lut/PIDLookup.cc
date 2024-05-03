// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Nathan Brei, Dmitry Kalinkin

#include <cmath>

#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>

#include "PIDLookup.h"

namespace eicrecon {

void PIDLookup::init(PIDLookupTable_service& lut_svc) {
  m_lut = lut_svc.load(m_cfg.filename);
  if (!m_lut) {
    throw std::runtime_error("LUT not available");
  }
}

void PIDLookup::process(const Input& input, const Output& output) const {
  const auto [recoparts_in, partassocs_in] = input;
  auto [recoparts_out]                     = output;

  for (const auto& recopart_without_pid : *recoparts_in) {

    edm4hep::MCParticle mcpart;
    auto recopart = recopart_without_pid.clone();

    bool assoc_found = false;
    for (auto assoc : *partassocs_in) {
      if (assoc.getRec() == recopart_without_pid) {
        assoc_found = true;
        mcpart      = assoc.getSim();
        break;
      }
    }
    if (not assoc_found) {
      recoparts_out->push_back(recopart);
      continue;
    }

    int true_pdg    = mcpart.getPDG();
    int charge      = recopart.getCharge();
    double momentum = edm4hep::utils::magnitude(recopart.getMomentum());

    // TODO: I'm still confused as to whether our lookup table actually contains eta vs theta.
    double eta   = edm4hep::utils::eta(recopart.getMomentum());
    double theta = edm4hep::utils::anglePolar(recopart.getMomentum()) / M_PI * 180.;
    double phi   = edm4hep::utils::angleAzimuthal(recopart.getMomentum()) / M_PI * 180.;

    auto entry = m_lut->Lookup(true_pdg, charge, momentum, theta, phi);

    int identified_pdg = 0; // unknown

    if (entry != nullptr) {

      double random_unit_interval = m_dist(m_gen);

      if (random_unit_interval < entry->prob_electron) {
        identified_pdg = 11; // electron
      } else if (random_unit_interval < (entry->prob_electron + entry->prob_pion)) {
        identified_pdg = 211; // pion
      } else if (random_unit_interval <
                 (entry->prob_electron + entry->prob_pion + entry->prob_kaon)) {
        identified_pdg = 321; // kaon
      } else if (random_unit_interval < (entry->prob_electron + entry->prob_pion +
                                         entry->prob_kaon + entry->prob_electron)) {
        identified_pdg = 2212; // proton
      } else {
        identified_pdg = 0; // unknown
        // If the lookup table contains rows where all probabilities are zero, the control flow ends
        // up here
      }
      if (charge < 0) {
        identified_pdg *= -1;
        // We want the identified PDG to have the same sign as the charge
      }
    }

    recopart.setPDG(identified_pdg);

    recoparts_out->push_back(recopart);
  }
}
} // namespace eicrecon
