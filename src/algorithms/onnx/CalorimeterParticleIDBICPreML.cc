// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa, Wouter Deconinck

#include "CalorimeterParticleIDBICPreML.h"

#include <edm4eic/CalorimeterHit.h>
#include <edm4hep/Vector3f.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace eicrecon {

namespace {

  struct SimpleHit {
    int layer;
    float e;
    float x;
    float y;
    float z;
  };

  static constexpr float kPi = 3.14159265358979323846F;

  float wrapDeltaPhi(float dphi) {
    if (dphi < -kPi) {
      dphi += 2.F * kPi;
    }
    if (dphi > kPi) {
      dphi -= 2.F * kPi;
    }
    return dphi;
  }

  float radial3D(float x, float y, float z) { return std::sqrt(x * x + y * y + z * z); }

  float etaFromXYZ(float x, float y, float z) {
    const float r     = radial3D(x, y, z);
    const float theta = std::atan2(r, z);
    return -std::log(std::tan(theta * 0.5F));
  }

  float phiFromXYZ(float x, float y) { return std::atan2(y, x); }

  void fillBranchTensor(const edm4eic::Cluster& cluster, std::vector<float>& eventTensor,
                        int nLayers, int nHits, int layerOffset, float r0Min, float r0Max,
                        float etaMin, float etaMax, float phiMin, float phiMax, bool zeroEta,
                        float lval) {
    std::vector<SimpleHit> hits;
    hits.reserve(cluster.getHits().size());

    float totalE = 0.F;
    for (auto const& h : cluster.getHits()) {
      const float e  = h.getEnergy();
      const auto pos = h.getPosition();
      hits.push_back({h.getLayer(), e, pos.x, pos.y, pos.z});
      totalE += e;
    }

    if (hits.empty() || totalE <= 0.F) {
      return;
    }

    for (auto& h : hits) {
      h.e /= totalE;
    }

    float wsum = 0.F;
    float xc   = 0.F;
    float yc   = 0.F;
    float zc   = 0.F;
    for (auto const& h : hits) {
      const float w = std::max(0.F, std::log(h.e) + 5.6F);
      wsum += w;
      xc += h.x * w;
      yc += h.y * w;
      zc += h.z * w;
    }

    if (wsum > 0.F) {
      xc /= wsum;
      yc /= wsum;
      zc /= wsum;
    }

    const float etaC = etaFromXYZ(xc, yc, zc);
    const float phiC = phiFromXYZ(xc, yc);

    std::vector<std::vector<SimpleHit>> buckets(nLayers);
    for (auto const& h : hits) {
      const int globalLayer = layerOffset + h.layer - 1;
      if (globalLayer >= 0 && globalLayer < nLayers) {
        buckets[globalLayer].push_back(h);
      }
    }

    for (auto& bucket : buckets) {
      std::sort(bucket.begin(), bucket.end(),
                [](auto const& a, auto const& b) { return a.e > b.e; });
    }

    const int nFeat = 5;
    auto setFeat    = [&](int layer, int hit, int feat, float value) {
      const std::size_t idx = ((static_cast<std::size_t>(layer) * nHits + hit) * nFeat + feat);
      eventTensor[idx]      = value;
    };

    for (int l = 0; l < nLayers; ++l) {
      auto const& bucket = buckets[l];
      const int keep     = std::min(static_cast<int>(bucket.size()), nHits);
      for (int h = 0; h < keep; ++h) {
        auto const& hit = bucket[h];

        const float rHit   = radial3D(hit.x, hit.y, hit.z);
        const float etaHit = etaFromXYZ(hit.x, hit.y, hit.z);
        const float phiHit = phiFromXYZ(hit.x, hit.y);

        const float rNorm = std::clamp((rHit - r0Min) / (r0Max - r0Min), 0.F, 1.F);

        float etaNorm = 0.F;
        if (!zeroEta) {
          etaNorm = std::clamp((etaHit - etaC - etaMin) / (etaMax - etaMin), 0.F, 1.F);
        }

        const float dsphi   = std::sin(0.5F * wrapDeltaPhi(phiHit - phiC));
        const float phiNorm = std::clamp((dsphi - phiMin) / (phiMax - phiMin), 0.F, 1.F);

        setFeat(l, h, 0, hit.e);
        setFeat(l, h, 1, rNorm);
        setFeat(l, h, 2, etaNorm);
        setFeat(l, h, 3, phiNorm);
        setFeat(l, h, 4, lval);
      }
    }
  }

} // namespace

void CalorimeterParticleIDBICPreML::init() {
  // Nothing
}

void CalorimeterParticleIDBICPreML::process(
    const CalorimeterParticleIDBICPreML::Input& input,
    const CalorimeterParticleIDBICPreML::Output& output) const {

  const auto [merged_clusters, imaging_clusters, scifi_clusters] = input;
  auto [feature_tensors]                                         = output;

  struct BICCandidate {
    const edm4eic::Cluster* imaging = nullptr;
    const edm4eic::Cluster* scifi   = nullptr;
  };
  std::vector<BICCandidate> candidates;
  candidates.reserve(merged_clusters->size());

  for (auto const& merged : *merged_clusters) {
    BICCandidate candidate;
    for (auto const& child : merged.getClusters()) {
      for (auto const& img : *imaging_clusters) {
        if (child == img) {
          candidate.imaging = &img;
        }
      }
      for (auto const& scfi : *scifi_clusters) {
        if (child == scfi) {
          candidate.scifi = &scfi;
        }
      }
    }
    if (candidate.imaging == nullptr || candidate.scifi == nullptr) {
      error("Merged BIC cluster {} does not contain both imaging and E/p-selected SciFi children",
            merged.getObjectID().index);
      throw std::runtime_error("Invalid BIC energy-position merged cluster");
    }
    candidates.push_back(candidate);
  }

  auto ft = feature_tensors->create();
  ft.addToShape(candidates.size());
  ft.addToShape(m_cfg.nLayers);
  ft.addToShape(m_cfg.nHits);
  ft.addToShape(5);
  ft.setElementType(1); // float

  for (auto const& candidate : candidates) {
    std::vector<float> eventTensor(static_cast<std::size_t>(m_cfg.nLayers) * m_cfg.nHits * 5, 0.F);
    fillBranchTensor(*candidate.imaging, eventTensor, m_cfg.nLayers, m_cfg.nHits, 0, m_cfg.r0Min,
                     m_cfg.r0Max, m_cfg.etaMin, m_cfg.etaMax, m_cfg.phiMin, m_cfg.phiMax, false,
                     0.F);
    fillBranchTensor(*candidate.scifi, eventTensor, m_cfg.nLayers, m_cfg.nHits,
                     m_cfg.scifiLayerOffset, m_cfg.r0Min, m_cfg.r0Max, m_cfg.etaMin, m_cfg.etaMax,
                     m_cfg.phiMin, m_cfg.phiMax, true, 1.F);

    for (float v : eventTensor) {
      ft.addToFloatData(v);
    }
  }

  std::size_t expected = 1;
  for (auto dim : ft.getShape()) {
    expected *= dim;
  }
  if (ft.floatData_size() != expected) {
    this->error("BIC CNN tensor size {} != {}", ft.floatData_size(), expected);
    throw std::runtime_error("BIC CNN tensor size mismatch");
  }
}

} // namespace eicrecon
