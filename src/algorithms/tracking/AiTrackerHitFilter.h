// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, EIC AI background filter proof-of-work
//
// AiTrackerHitFilter: splits a tracker-hit collection into "signal" and
// "noise" subset collections using ML pipeline
//   stage 1  locate the physics-interaction time t0 the frame
//   stage 2  score every hit as background probability
//   decide   keep hit iff score < threshold (separate in/out thresholds)
//
// Inputs:
//   1. all tracker hits of the frame
//      the models were trained on hits from ALL tracker systems incl. far-forward
//   2. the hits to actually split (the central-tracking subset)
//   3. raw-hit associations (rawHit<->simHit) - used only in "ideal" mode
// Outputs: signal hits, noise hits (subset collections of input 2)

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <onnxruntime_cxx_api.h>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/tracking/AiTrackerHitFilterConfig.h"

namespace eicrecon {

using AiTrackerHitFilterAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TrackerHitCollection, edm4eic::TrackerHitCollection,
                      edm4eic::MCRecoTrackerHitAssociationCollection>,
    algorithms::Output<edm4eic::TrackerHitCollection, edm4eic::TrackerHitCollection>>;

class AiTrackerHitFilter : public AiTrackerHitFilterAlgorithm,
                           public WithPodConfig<AiTrackerHitFilterConfig> {

public:
  AiTrackerHitFilter(std::string_view name)
      : AiTrackerHitFilterAlgorithm{
            name,
            {"inputAllHits", "inputHitsToFilter", "inputRawHitAssociations"},
            {"outputSignalHits", "outputNoiseHits"},
            "ML split of tracker hits into signal and background-noise collections"} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  // Plain view of one hit used by both stages
  struct HitView {
    float x, y, z, t; // mm, ns
    int64_t sys_idx;  // dense system index 0..21, -1 if unknown to the models
    uint64_t raw_id;  // packed podio ObjectID of the related raw hit (0 if none)
    uint64_t hit_id;  // packed podio ObjectID of the hit itself
  };

  double findT0Finder(const std::vector<HitView>& hits) const;
  double findT0Ideal(const std::vector<HitView>& hits,
                     const edm4eic::MCRecoTrackerHitAssociationCollection& assocs,
                     bool& found) const;

  // Runs watt (in-window, use_dt=true) or mlp (out-window) on a hit subset;
  // returns background probabilities aligned with `indices`.
  std::vector<float> scoreHits(Ort::Session& session, const std::vector<HitView>& hits,
                               const std::vector<std::size_t>& indices, double t0,
                               bool use_dt) const;

  mutable Ort::Env m_env{nullptr};
  mutable Ort::Session m_finder_session{nullptr};
  mutable Ort::Session m_watt_session{nullptr};
  mutable Ort::Session m_mlp_session{nullptr};

  // raw ePIC system id (cellID & 0xFF) -> dense embedding index 0..21
  std::unordered_map<int, int64_t> m_system_to_idx;
};

} // namespace eicrecon
