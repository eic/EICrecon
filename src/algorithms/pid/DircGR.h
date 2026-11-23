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
// STATUS (Phase 3, in progress): LUT + chromatic correction loading, the
// per-track gates, the photon loop (sim input), the thetaC estimators and
// the output filling (Gaussian lnL per hypothesis) are done. Raw photon
// input is not implemented yet.

#pragma once

#include <DD4hep/Segmentations.h>
#include <algorithms/algorithm.h>
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <edm4eic/IrtRadiatorInfoCollection.h>
#include <edm4eic/MCRecoTrackerHitLinkCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/pid/DircGRChrCorr.h"
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

/// Everything the photon loop needs about one track (one TrackSegment),
/// computed once per track. Port of the per-incident-particle block of the
/// old DIRCgr::Process. Angles in rad unless noted, lengths mm, times ns.
struct DircGRTrackGates {
  int box{-1}, bar{-1};
  double ptot{0};                // |p| at bar entry [GeV]
  std::vector<double> thetaCexp; // per hypothesis; 0 if below threshold
  double thetaCexpMax{0};        // max over hypotheses above threshold
  double detTheta{0}, detPhi{0}; // polar/azimuth of the entry POSITION
  double eta{0};
  std::array<double, 3> dirRot{}; // track direction, rotated into box frame
  double zIn{0}, tIn{0};
  // earliest photon arrival at the readout: direct (1) / via mirror (2);
  // "OP" = for the steepest photon allowed by the largest expected thetaC
  double tbeg1{0}, tbeg2{0}, tbeg1OP{0}, tbeg2OP{0};
  bool use1{false}, use2{false};          // photon directions allowed
  double cosAngLower{0}, cosAngUpper{0};  // cos(LUT polar) envelope at zIn
  bool hasChr{false};
  std::array<double, DircGRChrCorr::kNPar> chr{}; // pol1 pars at detTheta
};

/// Per-track result of the photon loop (old DIRCgr::Process, per incident
/// particle). Angles in mrad (as the old code's histograms).
struct DircGRTrackResult {
  int nHitsBox{0};    // photon hits in the track's bar box
  int nHitsPix{0};    // ... with a pixel inside the LUT grid
  int nPairsTight{0}; // (hit, LUT path) pairs passing the tight time gate
  int nPairsGated{0}; // ... and the cos(angle) envelope = used
  int nPhotonsUsed{0}; // distinct hits with >= 1 used path
  double thetaCMean{0}; // PID#1: mean over used pairs [mrad]
  double thetaCPeak{0}; // PID#2: peak-window mean [mrad] (the one used)
  std::vector<int> nBestHyp; // used pairs per best hypothesis
  // per used (hit, LUT path) pair, grouped by hit in hit order: thetaC
  // (chromatic-corrected, of the pair's best hypothesis) and azimuth around
  // the track (0 = towards the bar axis +z), both [rad]
  std::vector<std::pair<double, double>> thetaPhi;
};

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

  /// per-track gates from the bar-entry point (TrackPoint::surface must be
  /// the quartz-hit cellID). Returns false if the point is unusable.
  bool computeGates(const edm4eic::TrackPoint& pt, DircGRTrackGates& g) const;

  /// Tight time gate (old OPTimeCutTight). Returns -1 (direct, towards the
  /// prism) or +1 (via the mirror) if passed; -9 no match, -8 / -7 matched
  /// direction 1 / 2 but earlier than tbeg1OP / tbeg2OP.
  int timeGateTight(double hitt, double texp1, double texp2, const DircGRTrackGates& g) const;
  /// Crude +-crudeGateHalfWidth gate (old OPTimeCutCrude), diagnostics only.
  int timeGateCrude(double hitt, double texp1, double texp2, const DircGRTrackGates& g) const;
  /// cos(LUT path polar angle) inside the envelope for direction zdir (+-1)
  static bool cosAngOK(double cosOPtheta, int zdir, const DircGRTrackGates& g);
  /// Photon loop for one track over box-local photon hits (sim input):
  /// pixel -> LUT paths -> expected times -> gates -> chromatic correction
  /// -> 8-fold ambiguity -> per-path thetaC; then the per-track estimators.
  struct PhotonHit {
    double x, y, z, t; // global position [mm], time [ns]
    std::uint64_t cellID;
    int box;
  };
  void photonLoop(const DircGRTrackGates& g, const std::vector<PhotonHit>& hits,
                  DircGRTrackResult& r) const;

  /// index into the cos(angle) tables for entry z [mm] (old kzFromZ)
  static int kzFromZ(double z);

private:
  enum class PhotonInput { Sim, Raw };
  PhotonInput m_photon_input{PhotonInput::Sim};
  std::shared_ptr<const DircGRLUT> m_lut;
  std::shared_ptr<const DircGRChrCorr> m_chr;
  std::vector<double> m_hyp_mass; // [GeV], per configured hypothesis
  const dd4hep::DDSegmentation::BitFieldCoder* m_id_dec{nullptr};
  std::size_t m_module_idx{0}, m_bar_idx{0};
};

} // namespace eicrecon
