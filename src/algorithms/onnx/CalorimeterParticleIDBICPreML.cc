// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa, Wouter Deconinck

#include "CalorimeterParticleIDBICPreML.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace eicrecon {

static constexpr int kNLAYERS  = 12;
static constexpr int kNHITS    = 50;
static constexpr int kNFEAT    = 5;
static constexpr float R0_MIN  = 500.F;
static constexpr float R0_MAX  = 2000.F;
static constexpr float ETA_MIN = -0.3F;
static constexpr float ETA_MAX = 0.3F;
static constexpr float PHI_MIN = -0.4F;
static constexpr float PHI_MAX = 0.4F;
static constexpr float kPi     = 3.14159265358979323846F;

namespace {

  bool hasElectronPID(const edm4eic::Cluster& cl) {
    for (auto const& pid : cl.getParticleIDs()) {
      if (pid.getPDG() == 11) {
        return true;
      }
    }
    return false;
  }

} // namespace

void CalorimeterParticleIDBICPreML::init() {
  // Nothing
}

void CalorimeterParticleIDBICPreML::process(
    const CalorimeterParticleIDBICPreML::Input& input,
    const CalorimeterParticleIDBICPreML::Output& output) const {

  const auto [clusters, ep_pids] = input;
  auto [feature_tensors]         = output;

  // We do not need to read the PID collection contents directly here.
  // The relation is carried by cluster.getParticleIDs().
  // Keeping this input makes the dependency explicit in the chain.
  (void)ep_pids;

  std::vector<edm4eic::Cluster> sel_clusters;
  sel_clusters.reserve(clusters->size());

  for (auto const& cl : *clusters) {
    if (hasElectronPID(cl)) {
      sel_clusters.push_back(cl);
    }
  }

  auto ft = feature_tensors->create();
  ft.addToShape(sel_clusters.size());
  ft.addToShape(kNLAYERS);
  ft.addToShape(kNHITS);
  ft.addToShape(kNFEAT);
  ft.setElementType(1); // float

  for (auto const& cl : sel_clusters) {
    struct Hit {
      int layer;
      float e;
      float x;
      float y;
      float z;
    };

    std::vector<Hit> rec;
    rec.reserve(cl.getHits().size());

    float totalE = 0.f;
    for (auto const& h : cl.getHits()) {
      const float e  = h.getEnergy();
      const auto pos = h.getPosition();
      rec.push_back({h.getLayer(), e, pos.x, pos.y, pos.z});
      totalE += e;
    }

    if (totalE <= 0.f) {
      for (int l = 0; l < kNLAYERS; ++l) {
        for (int h = 0; h < kNHITS; ++h) {
          for (int f = 0; f < kNFEAT; ++f) {
            ft.addToFloatData(0.f);
          }
        }
      }
      continue;
    }

    for (auto& hit : rec) {
      hit.e /= totalE;
    }

    float wsum = 0.f;
    float xc   = 0.f;
    float yc   = 0.f;
    float zc   = 0.f;

    for (auto const& hit : rec) {
      const float lw = std::max(0.f, std::log(hit.e) + 5.6f);
      wsum += lw;
      xc += hit.x * lw;
      yc += hit.y * lw;
      zc += hit.z * lw;
    }

    if (wsum > 0.f) {
      xc /= wsum;
      yc /= wsum;
      zc /= wsum;
    }

    const float phi_c   = std::atan2(yc, xc);
    const float r_c     = std::hypot(xc, yc, zc);
    const float theta_c = std::atan2(r_c, zc);
    const float eta_c   = -std::log(std::tan(theta_c * 0.5f));

    std::vector<std::vector<Hit>> buckets(kNLAYERS);
    for (auto const& hit : rec) {
      const int idx = hit.layer - 1;
      if (idx >= 0 && idx < kNLAYERS) {
        buckets[idx].push_back(hit);
      }
    }

    for (auto& b : buckets) {
      std::sort(b.begin(), b.end(), [](auto const& a, auto const& b) { return a.e > b.e; });
    }

    for (int l = 0; l < kNLAYERS; ++l) {
      auto& bucket = buckets[l];
      for (int h = 0; h < kNHITS; ++h) {
        if (h < static_cast<int>(bucket.size())) {
          const auto& hit = bucket[h];

          const float r_hit     = std::hypot(hit.x, hit.y);
          const float theta_hit = std::atan2(r_hit, hit.z);
          const float eta_hit   = -std::log(std::tan(theta_hit * 0.5f));
          const float phi_hit   = std::atan2(hit.y, hit.x);

          const float r_norm = std::clamp((r_hit - R0_MIN) / (R0_MAX - R0_MIN), 0.f, 1.f);
          const float eta_norm =
              std::clamp((eta_hit - eta_c - ETA_MIN) / (ETA_MAX - ETA_MIN), 0.f, 1.f);

          float dphi = phi_hit - phi_c;
          if (dphi < -kPi) {
            dphi += 2.f * kPi;
          }
          if (dphi > kPi) {
            dphi -= 2.f * kPi;
          }

          const float dsphi    = std::sin(dphi * 0.5f);
          const float phi_norm = std::clamp((dsphi - PHI_MIN) / (PHI_MAX - PHI_MIN), 0.f, 1.f);

          ft.addToFloatData(hit.e);    // eh
          ft.addToFloatData(r_norm);   // r0
          ft.addToFloatData(eta_norm); // eta
          ft.addToFloatData(phi_norm); // phi
          ft.addToFloatData(0.f);      // lval
        } else {
          for (int f = 0; f < kNFEAT; ++f) {
            ft.addToFloatData(0.f);
          }
        }
      }
    }
  }

  std::size_t expected = 1;
  for (auto dim : ft.getShape()) {
    expected *= dim;
  }

  if (ft.floatData_size() != expected) {
    this->error("CNN tensor size {} != {}", ft.floatData_size(), expected);
    throw std::runtime_error("CNN tensor size mismatch");
  }
}

} // namespace eicrecon
