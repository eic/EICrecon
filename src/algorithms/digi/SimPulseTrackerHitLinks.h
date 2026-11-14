// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Link a raw tracker hit to the sim hits recorded in a SimPulse

#pragma once

#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/MCRecoTrackerHitLinkCollection.h>
#include <edm4eic/RawTrackerHit.h>
#include <edm4eic/SimPulse.h>
#include <edm4hep/SimTrackerHit.h>
#include <podio/RelationRange.h>

namespace eicrecon {

// Create one link and one association per sim hit of the pulse. The weight is the
// sim hit's share of the deposited energy, which is the pulse charge source in
// PulseGeneration<edm4hep::SimTrackerHit>, so the weights of a raw hit sum to 1.
inline void
addSimPulseTrackerHitLinks(const edm4eic::RawTrackerHit& raw_hit, const edm4eic::SimPulse& pulse,
                           edm4eic::MCRecoTrackerHitLinkCollection& links,
                           edm4eic::MCRecoTrackerHitAssociationCollection& associations) {
  double edep_sum = 0;
  for (const auto& sim_hit : pulse.getTrackerHits()) {
    edep_sum += sim_hit.getEDep();
  }
  const auto n_hits = pulse.trackerHits_size();
  for (const auto& sim_hit : pulse.getTrackerHits()) {
    const float weight = (edep_sum > 0) ? static_cast<float>(sim_hit.getEDep() / edep_sum)
                                        : 1.0F / static_cast<float>(n_hits);
    auto link          = links.create();
    link.setFrom(raw_hit);
    link.setTo(sim_hit);
    link.setWeight(weight);
    auto assoc = associations.create();
    assoc.setWeight(weight);
    assoc.setRawHit(raw_hit);
    assoc.setSimHit(sim_hit);
  }
}

} // namespace eicrecon
