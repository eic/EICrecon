// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 W.J. Llope
//
// hpDIRC geometric reconstruction (GR) of the Cherenkov angle.
//
// Inputs:
//   - TrackSegments at the bar entry (DircTruthTrackSegments, built from the
//     quartz-bar hits of the DIRCHits dual readout; TrackPoint::surface =
//     quartz-hit cellID -> module (bar box) / bar)
//   - MCP raw hits (DIRCRawHits from DircGRdig: cellID = MCP pixel,
//     timeStamp in units of timeResolution)
// Outputs (index-aligned with the input segments):
//   - CherenkovParticleID (hypotheses, npe, per-path theta/phi;
//     chargedParticle -> TrackSegment; rawHits unused)
//   - IrtRadiatorInfo (reconstructed angle [mrad], npe, nhits)
//
// Pixels: the LUT is generated on the DIRCHits MCP cellID grid, so the LUT
// pixel comes straight from the raw-hit cellID (grid size from the LUT files).
// Digitization (dead time, TDC, jitter) is done upstream in DircGRdig.

#pragma once

#include <DDSegmentation/BitFieldCoder.h>
#include <algorithms/algorithm.h>
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <edm4eic/IrtRadiatorInfoCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <array>
#include <cstddef>
#include <iterator>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/pid/DircGRConfig.h"
#include "services/dircgr/DircGRChrCorr.h"
#include "services/dircgr/DircGRLUT.h"

namespace eicrecon {

using DircGRAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TrackSegmentCollection, edm4eic::RawTrackerHitCollection>,
    algorithms::Output<edm4eic::CherenkovParticleIDCollection, edm4eic::IrtRadiatorInfoCollection>>;

/// Everything the photon loop needs about one track (one TrackSegment),
/// computed once per track. Angles in rad unless noted, lengths mm, times ns.
struct DircGRTrackGates {
  int box{-1}, bar{-1};          // from the cellID
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
  bool use1{false}, use2{false};         // photon directions allowed
  double cosAngLower{0}, cosAngUpper{0}; // cos(LUT path polar) envelope
  bool hasChr{false};
  std::array<double, DircGRChrCorr::kNPar> chr{}; // pol1 pars at detTheta
};

/// Per-track result of the photon loop. Angles in mrad unless noted.
struct DircGRTrackResult {
  int nHitsBox{0};           // pixel hits in the track's bar box
  int nHitsPix{0};           // ... with a pixel inside the LUT grid
  int nPairsTight{0};        // (hit, LUT path) pairs passing the tight time gate
  int nPairsGated{0};        // ... and the cos(angle) envelope = used
  int nPhotonsUsed{0};       // distinct hits with >= 1 used path
  double thetaCMean{0};      // mean over used pairs [mrad] (diagnostic)
  double thetaCPeak{0};      // peak-window mean [mrad], each photon weight 1: the track thetaC
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
                        {"inputTrackSegments", "inputRawHits"},
                        {"outputParticleIDs", "outputRadiatorInfo"},
                        "hpDIRC geometric reconstruction of the Cherenkov angle."} {}

  void init() final;
  void process(const Input&, const Output&) const final;

  /// per-track gates from the bar-entry point (TrackPoint::surface must be
  /// the quartz-hit cellID). Returns false if the point is unusable.
  bool computeGates(const edm4eic::TrackPoint& pt, DircGRTrackGates& g) const;

  /// Result of the tight time gate for one (hit, LUT path) pair
  enum class TimeGate {
    Direct,         // matches the direct path (towards the prism): zdir -1
    ViaMirror,      // matches the path via the mirror: zdir +1
    NoMatch,        // neither expected time matches
    DirectTooEarly, // matches direct, but earlier than the earliest allowed (tbeg1OP)
    MirrorTooEarly  // matches via mirror, but earlier than tbeg2OP
  };
  /// Tight time gate; if both directions match, via-mirror wins.
  TimeGate timeGateTight(double hitt, double texp1, double texp2, const DircGRTrackGates& g) const;
  /// cos(LUT path polar angle) inside the envelope for direction zdir (+-1)
  static bool cosAngOK(double cosOPtheta, int zdir, const DircGRTrackGates& g);
  /// A raw MCP hit on the LUT pixel grid
  struct PixelHit {
    int box;       // bar box (cellID module field)
    int ix, iy;    // LUT pixel indices; -1 if outside the LUT grid
    double t;      // [ns], TDC bin centre
  };
  /// Raw hits of one event -> LUT pixels and times
  std::vector<PixelHit> pixelHits(const edm4eic::RawTrackerHitCollection& hits) const;

  /// Photon loop for one track over the pixel hits: LUT paths -> expected
  /// times -> gates -> chromatic correction -> 8-fold ambiguity -> per-path
  /// thetaC; then the per-track estimators.
  void photonLoop(const DircGRTrackGates& g, const std::vector<PixelHit>& hits,
                  DircGRTrackResult& r) const;

private:
  // shared read-only tables, owned by DircGRCalibSvc
  const DircGRLUT* m_lut{nullptr};
  const DircGRChrCorr* m_chr{nullptr};
  std::vector<double> m_hyp_mass; // [GeV], per configured hypothesis
  const dd4hep::DDSegmentation::BitFieldCoder* m_id_dec{nullptr};
  std::size_t m_module_idx{0}, m_bar_idx{0}, m_x_idx{0}, m_y_idx{0};
};

} // namespace eicrecon
