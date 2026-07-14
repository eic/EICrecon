// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, EIC AI background filter proof-of-work
//

#include <edm4eic/RawTrackerHit.h>
#include <edm4hep/MCParticle.h>
#include <edm4hep/SimTrackerHit.h>
#include <fmt/core.h>
#include <onnxruntime_cxx_api.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>

#include "algorithms/tracking/AiTrackerHitFilter.h"

namespace eicrecon {

namespace {

// ---- time/space grid of the finder image (spec section 1) ----
constexpr double BIN_NS   = 25.0;
constexpr double T_MIN_NS = -100.0;
constexpr int N_TIME_BINS = 88;
constexpr int N_SPACE     = 64;
constexpr double Z_MIN = -2500.0, Z_MAX = 8000.0; // mm
constexpr double R_MIN = 0.0, R_MAX = 800.0;      // mm

// TODO: needs better configuration than this. Need to think how to go with this
// Dense system-index vocabulary of the hit-scorer embeddings
// Position in this sorted list = embedding index. Raw id = cellID & 0xFF.
constexpr std::array<int, 22> SYSTEM_IDS = {31, 59,  60,  61,  64,  68,  69,  70,
                                            72, 77,  78,  79,  82,  92,  122, 150,
                                            155, 156, 159, 160, 161, 162};

uint64_t pack_id(const podio::ObjectID& oid) {
  return (static_cast<uint64_t>(static_cast<uint32_t>(oid.collectionID)) << 32) |
         static_cast<uint32_t>(oid.index);
}

/// Origin band of a MCParticle under the background-merger convention:
/// generatorStatus 1/2 = signal, >=1000 = background band; Geant4 secondaries
/// (status 0) inherit the band of their first generator ancestor.
/// Returns 1 signal, 2 g4-from-signal, 3 background, 4 g4-from-background, 0 unknown.
int32_t get_origin_status(const edm4hep::MCParticle& particle) {
  auto classify = [](int32_t s) -> int32_t {
    if (s == 1 || s == 2) {
      return 1;
    }
    if (s >= 1000) {
      return 3;
    }
    return 0;
  };
  const int32_t gen_status = particle.getGeneratorStatus();
  if (gen_status != 0) {
    return classify(gen_status);
  }
  edm4hep::MCParticle current = particle;
  const int max_depth         = 200; // guard against cyclic/broken parent links
  for (int depth = 0; depth < max_depth; ++depth) {
    if (current.parents_size() == 0) {
      break;
    }
    edm4hep::MCParticle parent = current.getParents(0);
    if (!parent.isAvailable()) {
      break;
    }
    const int32_t parent_status = parent.getGeneratorStatus();
    if (parent_status != 0) {
      const int32_t band = classify(parent_status);
      if (band == 1) {
        return 2;
      }
      if (band == 3) {
        return 4;
      }
      return 0;
    }
    current = parent;
  }
  return 0;
}

Ort::Session make_session(Ort::Env& env, const std::string& path) {
  Ort::SessionOptions opts;
  opts.SetInterOpNumThreads(1);
  opts.SetIntraOpNumThreads(1);
  return {env, path.c_str(), opts};
}

} // namespace

void AiTrackerHitFilter::init() {
  for (std::size_t i = 0; i < SYSTEM_IDS.size(); ++i) {
    m_system_to_idx[SYSTEM_IDS[i]] = static_cast<int64_t>(i);
  }

  if (m_cfg.windowMode == "off") {
    info("windowMode=off: passthrough, no ONNX models loaded");
    return;
  }
  if (m_cfg.windowMode != "finder" && m_cfg.windowMode != "ideal") {
    throw std::runtime_error(
        fmt::format("AiTrackerHitFilter: unknown windowMode '{}' (finder|ideal|off)",
                    m_cfg.windowMode));
  }

  m_env = Ort::Env(ORT_LOGGING_LEVEL_WARNING, name().data());
  try {
    if (m_cfg.windowMode == "finder") {
      m_finder_session = make_session(m_env, m_cfg.finderModelPath);
    }
    m_watt_session = make_session(m_env, m_cfg.wattModelPath);
    m_mlp_session  = make_session(m_env, m_cfg.mlpModelPath);
  } catch (const Ort::Exception& e) {
    error("failed to load ONNX models: {}", e.what());
    throw;
  }
  info("AiTrackerHitFilter mode={} thr_in={} thr_out={} window=[-{},+{}]ns", m_cfg.windowMode,
       m_cfg.thresholdInWindow, m_cfg.thresholdOutWindow, m_cfg.windowFrontNs, m_cfg.windowBackNs);
}

double AiTrackerHitFilter::findT0Finder(const std::vector<HitView>& hits) const {
  // 2-channel (z,t) + (r,t) log1p occupancy image, NCHW [1,2,64,88].
  // Out-of-range hits clip INTO edge bins on purpose (RomanPots at z=32m
  // land in the top z row) - never discard them.
  std::vector<float> image(static_cast<std::size_t>(2) * N_SPACE * N_TIME_BINS, 0.0F);
  auto at = [&image](int ch, int s, int tb) -> float& {
    return image[(static_cast<std::size_t>(ch) * N_SPACE + s) * N_TIME_BINS + tb];
  };
  for (const auto& h : hits) {
    const int tb = std::clamp(static_cast<int>((h.t - T_MIN_NS) / BIN_NS), 0, N_TIME_BINS - 1);
    const int zb =
        std::clamp(static_cast<int>((h.z - Z_MIN) / (Z_MAX - Z_MIN) * N_SPACE), 0, N_SPACE - 1);
    const double r = std::sqrt(static_cast<double>(h.x) * h.x + static_cast<double>(h.y) * h.y);
    const int rb =
        std::clamp(static_cast<int>((r - R_MIN) / (R_MAX - R_MIN) * N_SPACE), 0, N_SPACE - 1);
    at(0, zb, tb) += 1.0F;
    at(1, rb, tb) += 1.0F;
  }
  for (auto& v : image) {
    v = std::log1p(v);
  }

  const std::array<int64_t, 4> shape{1, 2, N_SPACE, N_TIME_BINS};
  Ort::MemoryInfo mem =
      Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);
  Ort::Value input = Ort::Value::CreateTensor<float>(mem, image.data(), image.size(), shape.data(),
                                                     shape.size());
  const char* in_names[]  = {"image"};
  const char* out_names[] = {"bin_logits"};
  auto outputs =
      m_finder_session.Run(Ort::RunOptions{nullptr}, in_names, &input, 1, out_names, 1);
  const float* logits = outputs[0].GetTensorData<float>();
  const int best      = static_cast<int>(std::distance(
      logits, std::max_element(logits, logits + N_TIME_BINS)));
  return T_MIN_NS + BIN_NS * best; // LEFT edge of the winning bin (spec section 1)
}

double AiTrackerHitFilter::findT0Ideal(
    const std::vector<HitView>& hits, const edm4eic::MCRecoTrackerHitAssociationCollection& assocs,
    bool& found) const {
  found = false;

  // rawHit id -> (origin band, particle creation time)
  struct Truth {
    int32_t origin;
    float prt_time;
  };
  std::unordered_map<uint64_t, Truth> truth;
  truth.reserve(assocs.size());
  for (const auto& assoc : assocs) {
    if (!assoc.getRawHit().isAvailable() || !assoc.getSimHit().isAvailable()) {
      continue;
    }
    const auto particle = assoc.getSimHit().getParticle();
    if (!particle.isAvailable()) {
      continue;
    }
    truth[pack_id(assoc.getRawHit().getObjectID())] = {get_origin_status(particle),
                                                       particle.getTime()};
  }

  // t0_event = earliest origin-1 hit time (fallback: origin-1/2); used only
  // by the per-event albedo rule below.
  double t0_event    = std::numeric_limits<double>::infinity();
  double t0_event_12 = std::numeric_limits<double>::infinity();
  for (const auto& h : hits) {
    auto it = truth.find(h.raw_id);
    if (it == truth.end()) {
      continue;
    }
    if (it->second.origin == 1) {
      t0_event = std::min(t0_event, static_cast<double>(h.t));
    }
    if (it->second.origin == 1 || it->second.origin == 2) {
      t0_event_12 = std::min(t0_event_12, static_cast<double>(h.t));
    }
  }
  if (!std::isfinite(t0_event)) {
    t0_event = t0_event_12;
  }
  if (!std::isfinite(t0_event)) {
    return 0.0; // frame without signal hits - no truth t0
  }

  // t0_true = earliest origin-1/2 hit, excluding albedo (late unreconstructable
  // neutron products): lateness > 100 ns per-particle OR > 300 ns per-event.
  double t0_true = std::numeric_limits<double>::infinity();
  for (const auto& h : hits) {
    auto it = truth.find(h.raw_id);
    if (it == truth.end() || (it->second.origin != 1 && it->second.origin != 2)) {
      continue;
    }
    const bool albedo =
        (h.t - it->second.prt_time > 100.0) || (static_cast<double>(h.t) - t0_event > 300.0);
    if (!albedo) {
      t0_true = std::min(t0_true, static_cast<double>(h.t));
    }
  }
  if (!std::isfinite(t0_true)) {
    return 0.0;
  }
  found = true;
  return t0_true;
}

std::vector<float> AiTrackerHitFilter::scoreHits(Ort::Session& session,
                                                 const std::vector<HitView>& hits,
                                                 const std::vector<std::size_t>& indices,
                                                 double t0, bool use_dt) const {
  const auto n = static_cast<int64_t>(indices.size());
  std::vector<float> feat(static_cast<std::size_t>(n) * 4);
  std::vector<int64_t> sys(n);
  for (int64_t i = 0; i < n; ++i) {
    const auto& h   = hits[indices[i]];
    feat[i * 4 + 0] = h.x;
    feat[i * 4 + 1] = h.y;
    feat[i * 4 + 2] = h.z;
    feat[i * 4 + 3] = use_dt ? static_cast<float>(h.t - t0) : h.t;
    sys[i]          = h.sys_idx;
  }
  const std::array<int64_t, 2> feat_shape{n, 4};
  const std::array<int64_t, 1> sys_shape{n};
  Ort::MemoryInfo mem =
      Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);
  std::array<Ort::Value, 2> inputs{
      Ort::Value::CreateTensor<float>(mem, feat.data(), feat.size(), feat_shape.data(),
                                      feat_shape.size()),
      Ort::Value::CreateTensor<int64_t>(mem, sys.data(), sys.size(), sys_shape.data(),
                                        sys_shape.size())};
  const char* in_names[]  = {"hits", "system"};
  const char* out_names[] = {"score"};
  auto outputs =
      session.Run(Ort::RunOptions{nullptr}, in_names, inputs.data(), 2, out_names, 1);
  const float* score = outputs[0].GetTensorData<float>();
  return {score, score + n};
}

void AiTrackerHitFilter::process(const Input& input, const Output& output) const {
  const auto [all_hits, hits_to_filter, assocs] = input;
  auto [signal_hits, noise_hits]                = output;

  signal_hits->setSubsetCollection();
  noise_hits->setSubsetCollection();

  if (m_cfg.windowMode == "off") {
    for (const auto& hit : *hits_to_filter) {
      signal_hits->push_back(hit);
    }
    return;
  }

  // ---- flatten the full frame once ----
  std::vector<HitView> hits;
  hits.reserve(all_hits->size());
  std::size_t n_unknown_sys = 0;
  for (const auto& hit : *all_hits) {
    HitView v;
    v.x              = hit.getPosition().x;
    v.y              = hit.getPosition().y;
    v.z              = hit.getPosition().z;
    v.t              = hit.getTime();
    const int sys_id = static_cast<int>(hit.getCellID() & 0xFF);
    auto it          = m_system_to_idx.find(sys_id);
    v.sys_idx        = (it == m_system_to_idx.end()) ? -1 : it->second;
    if (v.sys_idx < 0) {
      ++n_unknown_sys;
    }
    v.raw_id = hit.getRawHit().isAvailable() ? pack_id(hit.getRawHit().getObjectID()) : 0;
    v.hit_id = pack_id(hit.getObjectID());
    hits.push_back(v);
  }
  if (n_unknown_sys > 0) {
    warning("{} hits with system id outside the trained vocabulary - kept unscored",
            n_unknown_sys);
  }

  // ---- stage 1: t0 ----
  bool have_window = !hits.empty();
  double t0        = 0.0;
  if (have_window) {
    if (m_cfg.windowMode == "finder") {
      t0 = findT0Finder(hits);
    } else {
      t0 = findT0Ideal(hits, *assocs, have_window);
      if (!have_window) {
        debug("ideal mode: no truth t0 in this frame, all hits scored as out-of-window");
      }
    }
  }

  // ---- stage 2: score every model-known hit of the frame ----
  std::vector<std::size_t> in_idx, out_idx;
  for (std::size_t i = 0; i < hits.size(); ++i) {
    if (hits[i].sys_idx < 0) {
      continue; // out-of-vocabulary system: never scored, always kept
    }
    const double dt = hits[i].t - t0;
    if (have_window && dt >= -m_cfg.windowFrontNs && dt <= m_cfg.windowBackNs) {
      in_idx.push_back(i);
    } else {
      out_idx.push_back(i);
    }
  }

  // hit id -> keep decision (hits not in the map default to keep)
  std::unordered_map<uint64_t, bool> keep;
  keep.reserve(in_idx.size() + out_idx.size());
  if (!in_idx.empty()) {
    // one call per frame: the attention model couples the in-window hits
    const auto scores = scoreHits(m_watt_session, hits, in_idx, t0, /*use_dt=*/true);
    for (std::size_t k = 0; k < in_idx.size(); ++k) {
      keep[hits[in_idx[k]].hit_id] = scores[k] < m_cfg.thresholdInWindow;
    }
  }
  if (!out_idx.empty()) {
    const auto scores = scoreHits(m_mlp_session, hits, out_idx, t0, /*use_dt=*/false);
    for (std::size_t k = 0; k < out_idx.size(); ++k) {
      keep[hits[out_idx[k]].hit_id] = scores[k] < m_cfg.thresholdOutWindow;
    }
  }

  // ---- split the target collection ----
  std::size_t n_unmatched = 0;
  for (const auto& hit : *hits_to_filter) {
    auto it         = keep.find(pack_id(hit.getObjectID()));
    const bool is_signal = (it == keep.end()) ? true : it->second;
    if (it == keep.end()) {
      ++n_unmatched;
    }
    if (is_signal) {
      signal_hits->push_back(hit);
    } else {
      noise_hits->push_back(hit);
    }
  }
  if (n_unmatched > n_unknown_sys) {
    // unknown-system hits legitimately have no score; anything beyond that
    // means the all-hits input does not contain the filter target collection
    warning("{} hits to filter had no score (unknown systems: {}) - check that "
            "the all-hits input is a superset of the filter target",
            n_unmatched, n_unknown_sys);
  }

  debug("t0={:.1f}ns in-window {} out {} -> signal {} noise {} of {}", t0, in_idx.size(),
        out_idx.size(), signal_hits->size(), noise_hits->size(), hits_to_filter->size());
}

} // namespace eicrecon
