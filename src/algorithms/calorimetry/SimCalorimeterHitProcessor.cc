// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim, Sylvester Joosten, Derek Anderson, Wouter Deconinck

#include "SimCalorimeterHitProcessor.h"

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <Evaluator/DD4hepUnits.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <podio/RelationRange.h>
#include <podio/podioVersion.h>
#include <cmath>
#include <cstddef>
#include <functional>
#include <gsl/pointers>
#include <limits>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>

#include "algorithms/calorimetry/SimCalorimeterHitProcessorConfig.h"
#include "MCTools.h"

using namespace dd4hep;

// Define necessary hash functions
namespace std {

#if defined(podio_VERSION_MAJOR) && defined(podio_VERSION_MINOR)
#if podio_VERSION <= PODIO_VERSION(1, 2, 0)
// Hash for podio::ObjectID
template <> struct hash<podio::ObjectID> {
  size_t operator()(const podio::ObjectID& id) const noexcept {
    size_t h1 = std::hash<uint32_t>{}(id.collectionID);
    size_t h2 = std::hash<int>{}(id.index);
    return h1 ^ (h2 << 1);
  }
};

// Necessary to make MCParticle hashable
template <> struct hash<edm4hep::MCParticle> {
  size_t operator()(const edm4hep::MCParticle& p) const noexcept {
    const auto& id = p.getObjectID();
    return std::hash<podio::ObjectID>()(id);
  }
};
#endif // podio version check
#endif // defined(podio_VERSION_MAJOR) && defined(podio_VERSION_MINOR)

// Hash for tuple<edm4hep::MCParticle, uint64_t>
// --> not yet supported by any compiler at the moment
template <> struct hash<std::tuple<edm4hep::MCParticle, uint64_t, int>> {
  size_t operator()(const std::tuple<edm4hep::MCParticle, uint64_t, int>& key) const noexcept {
    const auto& [particle, cellID, timeID] = key;
    size_t h1                              = hash<edm4hep::MCParticle>{}(particle);
    size_t h2                              = hash<uint64_t>{}(cellID);
    size_t h3                              = hash<int>{}(timeID);
    return ((h1 ^ (h2 << 1)) >> 1) ^ (h3 << 1);
  }
};

} // namespace std

// unnamed namespace for internal utility
namespace {
class HitContributionAccumulator {
private:
  float m_energy{0};
  float m_avg_time{0};
  float m_min_time{std::numeric_limits<float>::max()};
  edm4hep::Vector3f m_avg_position{0, 0, 0};

public:
  void add(const float energy, const float time, const edm4hep::Vector3f& pos) {
    m_energy += energy;
    m_avg_time += energy * time;
    m_avg_position = m_avg_position + energy * pos;
    m_min_time     = (time < m_min_time) ? time : m_min_time;
  }
  float getEnergy() const { return m_energy; }
  float getAvgTime() const { return m_energy > 0 ? m_avg_time / m_energy : 0; }
  float getMinTime() const { return m_min_time; }
  edm4hep::Vector3f getAvgPosition() const {
    return m_energy > 0 ? m_avg_position / m_energy : edm4hep::Vector3f{0, 0, 0};
  }
};

} // namespace

namespace eicrecon {

void SimCalorimeterHitProcessor::init() {

  // readout checks
  if (m_cfg.readout.empty()) {
    error("readoutClass is not provided, it is needed to know the fields in readout ids");
    throw std::runtime_error("readoutClass is not provided");
  }

  // get decoders
  try {
    m_id_spec = m_geo.detector()->readout(m_cfg.readout).idSpec();
  } catch (...) {
    debug("Failed to load ID decoder for {}", m_cfg.readout);
    throw std::runtime_error(fmt::format("Failed to load ID decoder for {}", m_cfg.readout));
  }

  // get m_hit_id_mask for adding up hits with the same dimensions that are merged over
  {
    uint64_t id_inverse_mask = 0;
    if (!m_cfg.hitMergeFields.empty()) {
      for (auto& field : m_cfg.hitMergeFields) {
        id_inverse_mask |= m_id_spec.field(field)->mask();
      }
    }
    m_hit_id_mask = ~id_inverse_mask;
    debug("ID mask in {:s}: {:#064b}", m_cfg.readout, m_hit_id_mask.value());
  }

  // get m_contribution_id_mask for adding up contributions with the same dimensions that are merged over
  {
    uint64_t id_inverse_mask = 0;
    if (!m_cfg.contributionMergeFields.empty()) {
      for (auto& field : m_cfg.contributionMergeFields) {
        id_inverse_mask |= m_id_spec.field(field)->mask();
      }
      m_contribution_id_mask = ~id_inverse_mask;
    }
    debug("ID mask in {:s}: {:#064b}", m_cfg.readout, m_contribution_id_mask.value());
  }

  // get reference position for attenuating hits and contributions
  if (!m_cfg.attenuationReferencePositionName.empty()) {
    m_attenuationReferencePosition =
        m_geo.detector()->constant<double>(m_cfg.attenuationReferencePositionName) *
        edm4eic::unit::mm / dd4hep::mm;
  }
}

// Group contributions by (primary particle, cell ID), apply optional attenuation, and optionally merge into superhits
void SimCalorimeterHitProcessor::process(const SimCalorimeterHitProcessor::Input& input,
                                         const SimCalorimeterHitProcessor::Output& output) const {

  const auto [in_hits]              = input;
  auto [out_hits, out_hit_contribs] = output;

  // Map for staging output information. We have 2 levels of structure:
  //   - top level: (MCParticle, Merged Hit CellID)
  //   - second level: (Merged Contributions)
  // Ideally we would want immediately create our output objects and modify the
  // contributions when needed. That could reduce the following code to a single loop
  // (instead of 2 consecutive loops). However, this is not possible as we may have to merge
  // (hence modify) contributions which is not supported for PodIO VectorMembers. Using
  // reasonable contribution merging, at least the intermediary structure should be
  // quite a bit smaller than the original hit collection.
  using HitIndex = std::tuple<edm4hep::MCParticle, uint64_t /* cellID */, int /* timeID */>;
  std::unordered_map<HitIndex,
                     std::unordered_map<uint64_t /* cellID */, HitContributionAccumulator>>
      hit_map;

  for (const auto& ih : *in_hits) {
    // the cell ID of the new superhit we are making
    const uint64_t newhit_cellID =
        (ih.getCellID() & m_hit_id_mask.value() & m_contribution_id_mask.value());
    // the cell ID of this particular contribution (we are using contributions to store
    // the hits making up this "superhit" with more segmentation)
    const uint64_t newcontrib_cellID = (ih.getCellID() & m_contribution_id_mask.value());
    // Optional attenuation
    const double attFactor =
        m_attenuationReferencePosition ? get_attenuation(ih.getPosition().z) : 1.;
    // Use primary particle (traced back through parents) to group contributions
    for (const auto& contrib : ih.getContributions()) {
      edm4hep::MCParticle primary = MCTools::lookup_primary(contrib);
      const double propagationTime =
          m_attenuationReferencePosition
              ? std::abs(m_attenuationReferencePosition.value() - ih.getPosition().z) *
                    m_cfg.inversePropagationSpeed
              : 0.;
      const double totalTime  = contrib.getTime() + propagationTime + m_cfg.fixedTimeDelay;
      const int newhit_timeID = std::floor(totalTime / m_cfg.timeWindow);
      auto& hit_accum         = hit_map[{primary, newhit_cellID, newhit_timeID}][newcontrib_cellID];
      hit_accum.add(contrib.getEnergy() * attFactor, totalTime, ih.getPosition());
    }
  }

  // We now have our data structured as we want it, next we need to visit all hits again
  // and create our output structures
  for (const auto& [hit_idx, contribs] : hit_map) {

    auto out_hit = out_hits->create();

    const auto& [particle, cellID, timeID] = hit_idx;
    HitContributionAccumulator new_hit;
    for (const auto& [contrib_idx, contrib] : contribs) {
      // Aggregate contributions to for the global hit
      new_hit.add(contrib.getEnergy(), contrib.getMinTime(), contrib.getAvgPosition());
      // Now store the contribution itself
      auto out_hit_contrib = out_hit_contribs->create();
      out_hit_contrib.setTime(contrib.getMinTime());
      out_hit_contrib.setStepPosition(edm4hep::Vector3f{0, 0, contrib.getAvgPosition().z});
      out_hit_contrib.setParticle(particle);
      out_hit.addToContributions(out_hit_contrib);
    }
    out_hit.setCellID(cellID);
    out_hit.setEnergy(new_hit.getEnergy());
    out_hit.setPosition(edm4hep::Vector3f{0, 0, new_hit.getAvgPosition().z});
  }
}

double SimCalorimeterHitProcessor::get_attenuation(double zpos) const {
  double length = std::abs(m_attenuationReferencePosition.value() - zpos);
  double factor =
      m_cfg.attenuationParameters[0] * std::exp(-length / m_cfg.attenuationParameters[1]) +
      (1 - m_cfg.attenuationParameters[0]) * std::exp(-length / m_cfg.attenuationParameters[2]);
  return factor;
}
} // namespace eicrecon
