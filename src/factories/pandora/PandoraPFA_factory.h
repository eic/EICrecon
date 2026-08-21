// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 EICrecon authors

#pragma once

#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <string>

#include "algorithms/pandora/PandoraPFA.h"
#include "algorithms/pandora/PandoraPFAConfig.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class PandoraPFA_factory : public JOmniFactory<PandoraPFA_factory, PandoraPFAConfig> {
public:
  using AlgoT = eicrecon::PandoraPFA;

private:
  std::unique_ptr<AlgoT> m_algo;

  // Calorimeter hit inputs (multiple collections per subsystem)
  PodioInput<edm4eic::CalorimeterHit> m_ecal_barrel_imaging_input{this};
  PodioInput<edm4eic::CalorimeterHit> m_ecal_barrel_scfi_input{this};
  PodioInput<edm4eic::CalorimeterHit> m_ecal_endcap_n_input{this};
  PodioInput<edm4eic::CalorimeterHit> m_ecal_endcap_p_input{this};
  PodioInput<edm4eic::CalorimeterHit> m_hcal_barrel_input{this};
  PodioInput<edm4eic::CalorimeterHit> m_hcal_endcap_n_input{this};

  // Track projections input
  PodioInput<edm4eic::TrackSegment> m_track_segment_input{this};

  // Output particles
  PodioOutput<edm4eic::ReconstructedParticle> m_particles_output{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /*run_number*/, uint64_t /*event_number*/) {
    // Combine barrel imaging + scfi hits
    auto ecal_barrel_combined = std::make_unique<edm4eic::CalorimeterHitCollection>();
    for (const auto& hit : *m_ecal_barrel_imaging_input()) {
      ecal_barrel_combined->push_back(hit.clone());
    }
    for (const auto& hit : *m_ecal_barrel_scfi_input()) {
      ecal_barrel_combined->push_back(hit.clone());
    }

    // Combine endcap N + P hits
    auto ecal_endcap_combined = std::make_unique<edm4eic::CalorimeterHitCollection>();
    for (const auto& hit : *m_ecal_endcap_n_input()) {
      ecal_endcap_combined->push_back(hit.clone());
    }
    for (const auto& hit : *m_ecal_endcap_p_input()) {
      ecal_endcap_combined->push_back(hit.clone());
    }

    // Combine hcal barrel + endcap N hits
    auto hcal_barrel_combined = std::make_unique<edm4eic::CalorimeterHitCollection>();
    for (const auto& hit : *m_hcal_barrel_input()) {
      hcal_barrel_combined->push_back(hit.clone());
    }

    auto hcal_endcap_combined = std::make_unique<edm4eic::CalorimeterHitCollection>();
    for (const auto& hit : *m_hcal_endcap_n_input()) {
      hcal_endcap_combined->push_back(hit.clone());
    }

    m_algo->process({ecal_barrel_combined.get(), ecal_endcap_combined.get(),
                     hcal_barrel_combined.get(), hcal_endcap_combined.get(),
                     m_track_segment_input()},
                    {m_particles_output().get()});
  }
}; // end PandoraPFA_factory

} // namespace eicrecon
