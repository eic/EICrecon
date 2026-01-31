// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Nathan Brei, Dmitry Kalinkin

#include <algorithms/service.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <cmath>
#include <exception>
#include <gsl/pointers>
#include <random>
#include <stdexcept>
#include <vector>

#include "algorithms/pid_lut/PIDLookup.h"
#include "algorithms/pid_lut/PIDLookupConfig.h"
#include "services/pid_lut/PIDLookupTableSvc.h"

namespace eicrecon {

void PIDLookup::init() {

  try {
    m_system = m_detector->constant<int32_t>(m_cfg.system);
  } catch (const std::exception& e) {
    error("Failed to get {} from the detector: {}", m_cfg.system, e.what());
    throw std::runtime_error("Failed to get requested ID from the detector");
  }

  auto& serviceSvc = algorithms::ServiceSvc::instance();
  auto* lut_svc    = serviceSvc.service<PIDLookupTableSvc>("PIDLookupTableSvc");

  m_lut = lut_svc->load(m_cfg.filename,
                        {
                            .pdg_values                   = m_cfg.pdg_values,
                            .charge_values                = m_cfg.charge_values,
                            .momentum_edges               = m_cfg.momentum_edges,
                            .polar_edges                  = m_cfg.polar_edges,
                            .azimuthal_binning            = m_cfg.azimuthal_binning,
                            .azimuthal_bin_centers_in_lut = m_cfg.azimuthal_bin_centers_in_lut,
                            .momentum_bin_centers_in_lut  = m_cfg.momentum_bin_centers_in_lut,
                            .polar_bin_centers_in_lut     = m_cfg.polar_bin_centers_in_lut,
                            .use_radians                  = m_cfg.use_radians,
                            .missing_electron_prob        = m_cfg.missing_electron_prob,
                        });
  if (m_lut == nullptr) {
    throw std::runtime_error("LUT not available");
  }
}

void PIDLookup::process(const Input& input, const Output& output) const {
  const auto [headers, recoparts_in, partassocs_in] = input;
  auto [recoparts_out, partassocs_out, partids_out] = output;

  // local random generator
  auto seed = m_uid.getUniqueID(*headers, name());
  std::default_random_engine generator(seed);
  std::uniform_real_distribution<double> uniform;

  for (const auto& recopart_without_pid : *recoparts_in) {
    auto recopart = recopart_without_pid.clone();

    // Find MCParticle from associations and propagate the relevant ones further
    auto best_assoc = edm4eic::MCRecoParticleAssociation::makeEmpty();
    if (partassocs_in) {
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
    }
    if (not best_assoc.isAvailable()) {
      recoparts_out->push_back(recopart);
      continue;
    }

    edm4hep::MCParticle mcpart = best_assoc.getSim();

    int true_pdg    = mcpart.getPDG();
    int true_charge = mcpart.getCharge();
    int charge      = recopart.getCharge();
    double momentum = edm4hep::utils::magnitude(recopart.getMomentum());

    double theta = edm4hep::utils::anglePolar(recopart.getMomentum()) / M_PI * 180.;
    double phi   = edm4hep::utils::angleAzimuthal(recopart.getMomentum()) / M_PI * 180.;

    trace("lookup for true_pdg={}, true_charge={}, momentum={:.2f} GeV, polar={:.2f}, "
          "aziumthal={:.2f}",
          true_pdg, true_charge, momentum, theta, phi);
    const auto* entry = m_lut->Lookup(true_pdg, true_charge, momentum, theta, phi);

    int identified_pdg = 0; // unknown

    if ((entry != nullptr) && ((entry->prob_electron != 0.) || (entry->prob_pion != 0.) ||
                               (entry->prob_kaon != 0.) || (entry->prob_proton != 0.))) {
      double random_unit_interval = uniform(generator);

      trace("entry with e:pi:K:P={}:{}:{}:{}", entry->prob_electron, entry->prob_pion,
            entry->prob_kaon, entry->prob_proton);

      recopart.addToParticleIDs(
          partids_out->create(m_system,                                // std::int32_t type
                              std::copysign(11, -charge),              // std::int32_t PDG
                              0,                                       // std::int32_t algorithmType
                              static_cast<float>(entry->prob_electron) // float likelihood
                              ));
      recopart.addToParticleIDs(
          partids_out->create(m_system,                            // std::int32_t type
                              std::copysign(211, charge),          // std::int32_t PDG
                              0,                                   // std::int32_t algorithmType
                              static_cast<float>(entry->prob_pion) // float likelihood
                              ));
      recopart.addToParticleIDs(
          partids_out->create(m_system,                            // std::int32_t type
                              std::copysign(321, charge),          // std::int32_t PDG
                              0,                                   // std::int32_t algorithmType
                              static_cast<float>(entry->prob_kaon) // float likelihood
                              ));
      recopart.addToParticleIDs(
          partids_out->create(m_system,                              // std::int32_t type
                              std::copysign(2212, charge),           // std::int32_t PDG
                              0,                                     // std::int32_t algorithmType
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
                                         entry->prob_kaon + entry->prob_proton)) {
        identified_pdg = 2212; // proton
        recopart.setParticleIDUsed((*partids_out)[partids_out->size() - 1]);
      }
    }

    if (identified_pdg != 0) {
      recopart.setPDG(std::copysign(identified_pdg, (identified_pdg == 11) ? -charge : charge));
      recopart.setMass(m_particleSvc.particle(identified_pdg).mass);
      recopart.setEnergy(std::hypot(momentum, m_particleSvc.particle(identified_pdg).mass));
    }

    if (identified_pdg != 0) {
      trace("randomized PDG is {}", recopart.getPDG());
    }

    recoparts_out->push_back(recopart);
  }
}

} // namespace eicrecon
