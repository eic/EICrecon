// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Dmitry Kalinkin

#include <edm4eic/CalorimeterHit.h>
#include <edm4eic/EDM4eicVersion.h>
#include <fmt/format.h>
#include <podio/RelationRange.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <tuple>
#include <vector>

#if EDM4EIC_VERSION_MAJOR >= 8
#include <edm4hep/MCParticle.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>

#include "CalorimeterParticleIDPreML.h"

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

  edm4eic::MCRecoClusterParticleAssociation
  findBestAssoc(const edm4eic::Cluster& cluster,
                const edm4eic::MCRecoClusterParticleAssociationCollection* cluster_assocs) {
    edm4eic::MCRecoClusterParticleAssociation best_assoc;
    if (cluster_assocs == nullptr) {
      return best_assoc;
    }

    for (auto const& assoc : *cluster_assocs) {
      if (assoc.getRec() == cluster) {
        if ((!best_assoc.isAvailable()) || (assoc.getWeight() > best_assoc.getWeight())) {
          best_assoc = assoc;
        }
      }
    }
    return best_assoc;
  }

} // namespace

void CalorimeterParticleIDPreML::init() {
  // Nothing
}

void CalorimeterParticleIDPreML::process(const CalorimeterParticleIDPreML::Input& input,
                                         const CalorimeterParticleIDPreML::Output& output) const {

  const auto [clusters, cluster_assocs, ep_pids] = input;
  auto [feature_tensors, target_tensors]         = output;

  std::vector<edm4eic::Cluster> sel_clusters;
  sel_clusters.reserve(clusters->size());

  if (!ep_pids || ep_pids->empty()) {
    for (auto const& cl : *clusters) {
      sel_clusters.push_back(cl);
    }
  } else {
    for (auto const& cl : *clusters) {
      if (hasElectronPID(cl)) {
        sel_clusters.push_back(cl);
      }
    }
  }

  // Backward-compatible legacy tensor path:
  // if no E/p PID collection is provided, also emit the original [N,11] feature tensor.
  if (!ep_pids || ep_pids->empty()) {
    edm4eic::MutableTensor feature_tensor = feature_tensors->create();
    feature_tensor.addToShape(sel_clusters.size());
    feature_tensor.addToShape(11);    // p, E/p, azimuthal, polar, 7 shape parameters
    feature_tensor.setElementType(1); // 1 - float

    edm4eic::MutableTensor target_tensor;
    const bool make_targets = (cluster_assocs != nullptr && target_tensors != nullptr);
    if (make_targets) {
      target_tensor = target_tensors->create();
      target_tensor.addToShape(sel_clusters.size());
      target_tensor.addToShape(2);     // is pion, is electron
      target_tensor.setElementType(7); // 7 - int64
    }

    for (auto const& cluster : sel_clusters) {
      double momentum = NAN;

      auto best_assoc = findBestAssoc(cluster, cluster_assocs);
      if (best_assoc.isAvailable()) {
        momentum = edm4hep::utils::magnitude(best_assoc.getSim().getMomentum());
      } else {
        warning("Can't find association for cluster. Skipping...");
        continue;
      }

      feature_tensor.addToFloatData(momentum);
      feature_tensor.addToFloatData(cluster.getEnergy() / momentum);
      auto pos = cluster.getPosition();
      feature_tensor.addToFloatData(edm4hep::utils::anglePolar(pos));
      feature_tensor.addToFloatData(edm4hep::utils::angleAzimuthal(pos));
      for (std::size_t par_ix = 0; par_ix < cluster.shapeParameters_size(); ++par_ix) {
        feature_tensor.addToFloatData(cluster.getShapeParameters(par_ix));
      }

      if (make_targets) {
        int64_t is_electron = 0;
        int64_t is_pion     = 0;
        if (best_assoc.isAvailable()) {
          is_electron = static_cast<int64_t>(best_assoc.getSim().getPDG() == 11);
          is_pion     = static_cast<int64_t>(best_assoc.getSim().getPDG() != 11);
        }
        target_tensor.addToInt64Data(is_pion);
        target_tensor.addToInt64Data(is_electron);
      }
    }

    std::size_t expected_num_entries = feature_tensor.getShape(0) * feature_tensor.getShape(1);
    if (feature_tensor.floatData_size() != expected_num_entries) {
      error("Inconsistent output tensor shape and element count: {} != {}",
            feature_tensor.floatData_size(), expected_num_entries);
      throw std::runtime_error(
          fmt::format("Inconsistent output tensor shape and element count: {} != {}",
                      feature_tensor.floatData_size(), expected_num_entries));
    }
  }

  // CNN tensor path: [N, 12, 50, 5]
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
      float e  = h.getEnergy();
      auto pos = h.getPosition();
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
      float lw = std::max(0.f, std::log(hit.e) + 5.6f);
      wsum += lw;
      xc += hit.x * lw;
      yc += hit.y * lw;
      zc += hit.z * lw;
    }

    if (wsum > 0.f) {
      xc /= wsum;
      yc /= wsum;
      zc /= wsum;
    } else {
      xc = 0.f;
      yc = 0.f;
      zc = 0.f;
    }

    float phi_c   = std::atan2(yc, xc);
    float r_c     = std::hypot(xc, yc, zc);
    float theta_c = std::atan2(r_c, zc);
    float eta_c   = -std::log(std::tan(theta_c * 0.5f));

    std::vector<std::vector<Hit>> buckets(kNLAYERS);
    for (auto const& hit : rec) {
      int idx = hit.layer - 1;
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
          auto const& hit = bucket[h];

          float r_hit     = std::hypot(hit.x, hit.y);
          float theta_hit = std::atan2(r_hit, hit.z);
          float eta_hit   = -std::log(std::tan(theta_hit * 0.5f));
          float phi_hit   = std::atan2(hit.y, hit.x);

          float r_norm   = std::clamp((r_hit - R0_MIN) / (R0_MAX - R0_MIN), 0.f, 1.f);
          float eta_norm = std::clamp((eta_hit - eta_c - ETA_MIN) / (ETA_MAX - ETA_MIN), 0.f, 1.f);

          float dphi = phi_hit - phi_c;
          if (dphi < -kPi) {
            dphi += 2.f * kPi;
          }
          if (dphi > kPi) {
            dphi -= 2.f * kPi;
          }

          float dsphi    = std::sin(dphi * 0.5f);
          float phi_norm = std::clamp((dsphi - PHI_MIN) / (PHI_MAX - PHI_MIN), 0.f, 1.f);

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

  {
    auto const& shape    = ft.getShape();
    std::size_t expected = 1;
    for (auto dim : shape) {
      expected *= dim;
    }
    if (ft.floatData_size() != expected) {
      this->error("CNN tensor size {} != {}", ft.floatData_size(), expected);
      throw std::runtime_error("CNN tensor size mismatch");
    }
  }
}

} // namespace eicrecon
#endif
