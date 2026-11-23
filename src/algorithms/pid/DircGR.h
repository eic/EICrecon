// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 hpDIRC GR effort
//
// hpDIRC geometric reconstruction (GR) of the Cherenkov angle.
//
// Inputs:
//   - TrackSegments at the bar entry (truth stage: DircTruthTrackSegments;
//     TrackPoint::surface = quartz-hit cellID -> module (bar box) / bar)
//   - photons, one of (selected by config.photonInput):
//       "sim": MCP SimTrackerHits (DIRCMCPHits)
//       "raw": RawTrackerHits (DIRCRawHits) [+ links for truth diagnostics]
// Outputs (index-aligned with the input segments):
//   - CherenkovParticleID (hypotheses, npe, per-photon theta/phi, raw hits;
//     chargedParticle -> TrackSegment)
//   - IrtRadiatorInfo (reconstructed angle, npe, nhits)
//
// STATUS (Phase 2 skeleton): LUT loading + I/O plumbing only; the GR physics
// (Phase 3) is not ported yet, so all outputs are empty placeholders.

#pragma once

#include <DD4hep/Segmentations.h>
#include <algorithms/algorithm.h>
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <edm4eic/IrtRadiatorInfoCollection.h>
#include <edm4eic/MCRecoTrackerHitLinkCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/pid/DircGRConfig.h"
#include "algorithms/pid/DircGRLUT.h"

namespace eicrecon {

using DircGRAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TrackSegmentCollection,
                      std::optional<edm4hep::SimTrackerHitCollection>,
                      std::optional<edm4eic::RawTrackerHitCollection>,
                      std::optional<edm4eic::MCRecoTrackerHitLinkCollection>>,
    algorithms::Output<edm4eic::CherenkovParticleIDCollection,
                       edm4eic::IrtRadiatorInfoCollection>>;

class DircGR : public DircGRAlgorithm, public WithPodConfig<DircGRConfig> {
public:
  DircGR(std::string_view name)
      : DircGRAlgorithm{name,
                        {"inputTrackSegments", "inputSimPhotonHits", "inputRawPhotonHits",
                         "inputRawPhotonHitLinks"},
                        {"outputParticleIDs", "outputRadiatorInfo"},
                        "hpDIRC geometric reconstruction of the Cherenkov angle."} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  enum class PhotonInput { Sim, Raw };
  PhotonInput m_photon_input{PhotonInput::Sim};
  std::shared_ptr<const DircGRLUT> m_lut;
  const dd4hep::DDSegmentation::BitFieldCoder* m_id_dec{nullptr};
  std::size_t m_module_idx{0}, m_bar_idx{0};
};

} // namespace eicrecon
