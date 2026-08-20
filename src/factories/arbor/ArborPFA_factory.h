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

/// ArborPFA factory: tree-based particle flow for highly granular calorimeters.
/// ArborPFA runs the same core algorithm infrastructure as PandoraPFA but with
/// different clustering and tree-building strategies optimized for fine-grained
/// calorimeter geometries.
class ArborPFA_factory : public JOmniFactory<ArborPFA_factory, PandoraPFAConfig> {
public:
  using AlgoT = eicrecon::PandoraPFA;

private:
  std::unique_ptr<AlgoT> m_algo;

  // Calorimeter hit inputs
  PodioInput<edm4eic::CalorimeterHit> m_ecal_barrel_input{this};
  PodioInput<edm4eic::CalorimeterHit> m_ecal_endcap_input{this};
  PodioInput<edm4eic::CalorimeterHit> m_hcal_barrel_input{this};
  PodioInput<edm4eic::CalorimeterHit> m_hcal_endcap_input{this};

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
    m_algo->process({m_ecal_barrel_input(), m_ecal_endcap_input(), m_hcal_barrel_input(),
                     m_hcal_endcap_input(), m_track_segment_input()},
                    {m_particles_output().get()});
  }
}; // end ArborPFA_factory

} // namespace eicrecon
