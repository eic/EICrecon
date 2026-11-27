// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 W.J. Llope

// hpDIRC MCP photon digitization: SimTrackerHits (one per detected photon,
// cellID = MCP pixel) -> RawTrackerHits, one per pixel and dead-time window:
//   - optional Gaussian time jitter per photon,
//   - per pixel, photons in time order: the earliest starts a hit, photons
//     within deadTime of the hit start are merged into it, the next one after
//     that starts a new hit (deadTime 0, the default: every photon a hit),
//   - timeStamp = floor(t / timeResolution) (TDC), charge = merged photons.
// Quantum efficiency is applied in npsim (stacking action), not here.
// Truth: each raw hit is linked to all its photons; the photon that set the
// time comes first (weight 1/n each). Links only, no (legacy) associations.

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/MCRecoTrackerHitLinkCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <string>
#include <string_view>

#include "DircGRdigConfig.h"
#include "algorithms/interfaces/UniqueIDGenSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using DircGRdigAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::EventHeaderCollection, edm4hep::SimTrackerHitCollection>,
    algorithms::Output<edm4eic::RawTrackerHitCollection, edm4eic::MCRecoTrackerHitLinkCollection>>;

class DircGRdig : public DircGRdigAlgorithm, public WithPodConfig<DircGRdigConfig> {
public:
  DircGRdig(std::string_view name)
      : DircGRdigAlgorithm{name,
                           {"eventHeaderCollection", "inputHitCollection"},
                           {"outputRawHitCollection", "outputHitLinks"},
                           "hpDIRC MCP digitization: earliest photon per pixel, dead time, TDC."} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const algorithms::UniqueIDGenSvc& m_uid = algorithms::UniqueIDGenSvc::instance();
};

} // namespace eicrecon
