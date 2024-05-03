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
  auto [recoparts_out, partids_out]        = output;

  const double phi_upper_bound = m_lut->GetPhiBinning().upper_bound;

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
    int true_charge = mcpart.getCharge();
    int charge      = recopart.getCharge();
    double momentum = edm4hep::utils::magnitude(recopart.getMomentum());

    // TODO: I'm still confused as to whether our lookup table actually contains eta vs theta.
    double eta   = edm4hep::utils::eta(recopart.getMomentum());
    double theta = edm4hep::utils::anglePolar(recopart.getMomentum()) / M_PI * 180.;
    double phi   = edm4hep::utils::angleAzimuthal(recopart.getMomentum()) / M_PI * 180.;

    // Table doesn't discriminate between the charges
    if (m_lut->GetChargeBinning().size() == 1) {
      true_charge = m_lut->GetChargeBinning()[0];
    }

    // Azimuthal symmetry
    phi = std::fmod(phi, phi_upper_bound);

    auto entry = m_lut->Lookup(true_pdg, true_charge, momentum, theta, phi);

    int identified_pdg = 0; // unknown

    if ((entry != nullptr) && ((entry->prob_electron != 0.) || (entry->prob_pion != 0.) || (entry->prob_kaon != 0.) || (entry->prob_electron != 0.))) {

      double random_unit_interval = m_dist(m_gen);

      recopart.addToParticleIDs(partids_out->create(
        0,    // std::int32_t type
        11,   // std::int32_t PDG
        0,    // std::int32_t algorithmType
        static_cast<float>(entry->prob_electron) // float likelihood
      ));
      recopart.addToParticleIDs(partids_out->create(
        0,    // std::int32_t type
        211,  // std::int32_t PDG
        0,    // std::int32_t algorithmType
        static_cast<float>(entry->prob_pion) // float likelihood
      ));
      recopart.addToParticleIDs(partids_out->create(
        0,    // std::int32_t type
        321,  // std::int32_t PDG
        0,    // std::int32_t algorithmType
        static_cast<float>(entry->prob_kaon) // float likelihood
      ));
      recopart.addToParticleIDs(partids_out->create(
        0,    // std::int32_t type
        2212, // std::int32_t PDG
        0,    // std::int32_t algorithmType
        static_cast<float>(entry->prob_proton) // float likelihood
      ));

      if (random_unit_interval < entry->prob_electron) {
        identified_pdg = 11; // electron
        recopart.setParticleIDUsed((*partids_out)[partids_out->size() - 4]);
      } else if (random_unit_interval < (entry->prob_electron + entry->prob_pion)) {
        identified_pdg = 211; // pion
        recopart.setParticleIDUsed((*partids_out)[partids_out->size() - 3]);
      } else if (random_unit_interval <
                 (entry->prob_electron + entry->prob_pion + entry->prob_kaon)) {
        identified_pdg = 321; // kaon
        recopart.setParticleIDUsed((*partids_out)[partids_out->size() - 2]);
      } else if (random_unit_interval < (entry->prob_electron + entry->prob_pion +
                                         entry->prob_kaon + entry->prob_electron)) {
        identified_pdg = 2212; // proton
        recopart.setParticleIDUsed((*partids_out)[partids_out->size() - 1]);
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
