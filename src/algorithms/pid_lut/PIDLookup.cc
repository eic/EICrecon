// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Nathan Brei, Dmitry Kalinkin

#include <cmath>

#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>

#include "PIDLookup.h"

namespace eicrecon {

void PIDLookup::init() {
  auto& serviceSvc = algorithms::ServiceSvc::instance();
  auto lut_svc = serviceSvc.service<PIDLookupTableSvc>("PIDLookupTableSvc");

  m_lut = lut_svc->load(m_cfg.filename, {
    .pdg_values=m_cfg.pdg_values,
    .charge_values=m_cfg.charge_values,
    .momentum_edges=m_cfg.momentum_edges,
    .polar_edges=m_cfg.polar_edges,
    .azimuthal_binning=m_cfg.azimuthal_binning,
    .momentum_bin_centers_in_lut=m_cfg.momentum_bin_centers_in_lut,
    .polar_bin_centers_in_lut=m_cfg.polar_bin_centers_in_lut,
    .skip_legacy_header=m_cfg.skip_legacy_header,
    .use_radians=m_cfg.use_radians,
    .missing_electron_prob=m_cfg.missing_electron_prob,
  });
  if (m_lut == nullptr) {
    throw std::runtime_error("LUT not available");
  }
}

void PIDLookup::process(const Input& input, const Output& output) const {
  const auto [recoparts_in, partassocs_in] = input;
  auto [recoparts_out, partids_out]        = output;

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

    trace("lookup for true_pdg={}, true_charge={}, momentum={:.2f} GeV, polar={:.2f}, aziumthal={:.2f}",
      true_pdg, true_charge, momentum, theta, phi);
    auto entry = m_lut->Lookup(true_pdg, true_charge, momentum, theta, phi);

    int identified_pdg = 0; // unknown

    if ((entry != nullptr) && ((entry->prob_electron != 0.) || (entry->prob_pion != 0.) || (entry->prob_kaon != 0.) || (entry->prob_electron != 0.))) {
      double random_unit_interval = m_dist(m_gen);

      trace("entry with e:pi:K:P={}:{}:{}:{}", entry->prob_electron, entry->prob_pion, entry->prob_kaon, entry->prob_proton);

      recopart.addToParticleIDs(partids_out->create(
        m_cfg.system,                // std::int32_t type
        std::copysign(11, charge),   // std::int32_t PDG
        0,                           // std::int32_t algorithmType
        static_cast<float>(entry->prob_electron) // float likelihood
      ));
      recopart.addToParticleIDs(partids_out->create(
        m_cfg.system,                // std::int32_t type
        std::copysign(211, charge),  // std::int32_t PDG
        0,                           // std::int32_t algorithmType
        static_cast<float>(entry->prob_pion) // float likelihood
      ));
      recopart.addToParticleIDs(partids_out->create(
        m_cfg.system,                // std::int32_t type
        std::copysign(321, charge),  // std::int32_t PDG
        0,                           // std::int32_t algorithmType
        static_cast<float>(entry->prob_kaon) // float likelihood
      ));
      recopart.addToParticleIDs(partids_out->create(
        m_cfg.system,                // std::int32_t type
        std::copysign(2212, charge), // std::int32_t PDG
        0,                           // std::int32_t algorithmType
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
    }

    recopart.setPDG(std::copysign(identified_pdg, charge));

    if (identified_pdg != 0) {
      trace("randomized PDG is {}", recopart.getPDG());
    }

    recoparts_out->push_back(recopart);
  }
}

} // namespace eicrecon
