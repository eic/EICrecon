// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Dmitry Kalinkin

#include <edm4eic/EDM4eicVersion.h>

#if EDM4EIC_VERSION_MAJOR >= 8
#include <boost/histogram.hpp>
#include <cstddef>
#include <cstdint>
#include <edm4hep/MCParticle.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <podio/podioVersion.h>
#include <gsl/pointers>
#include <stdexcept>

#include "DIRCParticleIDPreML.h"

namespace bh = boost::histogram;

namespace eicrecon {

void DIRCParticleIDPreML::init() {
  // id_spec = m_geo.detector()->readout(m_cfg.readout).idSpec();
  try {
    auto id_spec   = m_geo.detector()->readout("DIRCBarHits").idSpec();
    m_field_module = id_spec.field("module");
    m_field_x      = id_spec.field("x");
    m_field_y      = id_spec.field("y");
  } catch (std::exception&) {
    std::terminate();
  }
}

void DIRCParticleIDPreML::process(const DIRCParticleIDPreML::Input& input,
                                  const DIRCParticleIDPreML::Output& output) const {

  const auto [dirc_hits, tracks, track_assocs]                           = input;
  auto [dirc_feature_tensors, track_feature_tensors, pid_target_tensors] = output;

  edm4eic::MutableTensor track_feature_tensor;
  bool track_feature_tensor_available = false;

  edm4eic::MutableTensor pid_target_tensor;
  if (track_assocs) {
    pid_target_tensor = pid_target_tensors->create();
    pid_target_tensor.addToShape(tracks->size());
    pid_target_tensor.addToShape(2);     // is pion, is kaon
    pid_target_tensor.setElementType(7); // 7 - int64
  }

  //bh::axis::integer<> axis_mod(0, 11 + 1);
  bh::axis::integer<> axis_mod(0, 0 + 1); // for hackathon
  bh::axis::integer<> axis_x(-39, 40 + 1);
  bh::axis::integer<> axis_y(-57, 58 + 1);
  auto hit_map = bh::make_histogram(
    axis_mod,
    axis_x,
    axis_y
  );
  for (edm4eic::RawTrackerHit dirc_hit : *dirc_hits) {
    int64_t cell_id = dirc_hit.getCellID();

    hit_map(std::make_tuple<>(m_field_module->value(cell_id), m_field_x->value(cell_id),
                              m_field_y->value(cell_id)));
  }
  edm4eic::MutableTensor dirc_feature_tensor = dirc_feature_tensors->create();
  dirc_feature_tensor.setElementType(1); // 1 - float
  dirc_feature_tensor.addToShape(axis_mod.size());
  dirc_feature_tensor.addToShape(axis_x.size());
  dirc_feature_tensor.addToShape(axis_y.size());
  int sum = 0;
  for (bh::axis::index_type ix_mod = 0; ix_mod < axis_mod.size(); ix_mod++) {
    for (bh::axis::index_type ix_x = 0; ix_x < axis_x.size(); ix_x++) {
      for (bh::axis::index_type ix_y = 0; ix_y < axis_y.size(); ix_y++) {
        dirc_feature_tensor.addToFloatData(static_cast<float>(hit_map.at(ix_mod, ix_x, ix_y)));
        ++sum;
      }
    }
  }

  for (edm4eic::ReconstructedParticle track : *tracks) {
    if (not track_feature_tensor_available) {
      track_feature_tensor_available = true;
      track_feature_tensor = track_feature_tensors->create();
      track_feature_tensor.addToShape(tracks->size());
      track_feature_tensor.addToShape(3);     // p, polar, azimuthal
      track_feature_tensor.setElementType(1); // 1 - float
    }

    auto momentum = track.getMomentum();
    track_feature_tensor.addToFloatData(edm4hep::utils::magnitude(momentum));
    track_feature_tensor.addToFloatData(edm4hep::utils::anglePolar(momentum));
    track_feature_tensor.addToFloatData(edm4hep::utils::angleAzimuthal(momentum));

    if (track_assocs) {
#if podio_VERSION >= PODIO_VERSION(0, 17, 4)
      auto best_assoc = edm4eic::MCRecoParticleAssociation::makeEmpty();
#else
      edm4eic::MCRecoParticleAssociationCollection _best_assoc_coll;
      edm4eic::MCRecoParticleAssociation best_assoc = _best_assoc_coll.create();
      best_assoc.unlink();
#endif
      for (auto assoc : *track_assocs) {
        if (assoc.getRec() == track) {
          if ((not best_assoc.isAvailable()) || (assoc.getWeight() > best_assoc.getWeight())) {
            best_assoc = assoc;
          }
        }
      }

      edm4hep::MCParticle part;
      if (best_assoc.isAvailable()) {
        part = best_assoc.getSim();
      } else {
        debug("Can't find association for track. Skipping...");
        continue;
      }

      int64_t is_pion = std::abs(part.getPDG()) == 211;
      int64_t is_kaon = std::abs(part.getPDG()) == 321;
      pid_target_tensor.addToInt64Data(is_pion);
      pid_target_tensor.addToInt64Data(is_kaon);
    }
  }

  size_t expected_num_entries;
  expected_num_entries = dirc_feature_tensor.getShape(0) * dirc_feature_tensor.getShape(1) *
                         dirc_feature_tensor.getShape(2);
  if (dirc_feature_tensor.floatData_size() != expected_num_entries) {
    error("Inconsistent output tensor shape and element count: {} != {}",
          dirc_feature_tensor.floatData_size(), expected_num_entries);
    throw std::runtime_error(
        fmt::format("Inconsistent output tensor shape and element count: {} != {}",
                    dirc_feature_tensor.floatData_size(), expected_num_entries));
  }
  expected_num_entries = track_feature_tensor.getShape(0) * track_feature_tensor.getShape(1);
  if (track_feature_tensor.floatData_size() != expected_num_entries) {
    error("Inconsistent output tensor shape and element count: {} != {}",
          track_feature_tensor.floatData_size(), expected_num_entries);
    throw std::runtime_error(
        fmt::format("Inconsistent output tensor shape and element count: {} != {}",
                    track_feature_tensor.floatData_size(), expected_num_entries));
  }
}

} // namespace eicrecon
#endif
