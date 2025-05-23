// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Dmitry Kalinkin

#include <edm4eic/EDM4eicVersion.h>

#if EDM4EIC_VERSION_MAJOR >= 8
#include <cstddef>
#include <cstdint>
#include <edm4hep/MCParticle.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <cmath>
#include <stdexcept>

#include <gsl/pointers>

#include "CalorimeterParticleIDPreML.h"

namespace eicrecon {

void CalorimeterParticleIDPreML::init() {
  // Nothing
}

void CalorimeterParticleIDPreML::process(const CalorimeterParticleIDPreML::Input& input,
                                         const CalorimeterParticleIDPreML::Output& output) const {

  const auto [clusters, cluster_assocs]  = input;
  auto [feature_tensors, target_tensors] = output;

  edm4eic::MutableTensor feature_tensor = feature_tensors->create();
  feature_tensor.addToShape(clusters->size());
  feature_tensor.addToShape(11);    // p, E/p, azimuthal, polar, 7 shape parameters
  feature_tensor.setElementType(1); // 1 - float

  edm4eic::MutableTensor target_tensor;
  if (cluster_assocs != nullptr) {
    target_tensor = target_tensors->create();
    target_tensor.addToShape(clusters->size());
    target_tensor.addToShape(2);     // is electron, is hadron
    target_tensor.setElementType(7); // 7 - int64
  }

  for (edm4eic::Cluster cluster : *clusters) {
    double momentum = NAN;
    {
      // FIXME: use track momentum once matching to tracks becomes available
      edm4eic::MCRecoClusterParticleAssociation best_assoc;
      for (auto assoc : *cluster_assocs) {
        if (assoc.getRec() == cluster) {
          if ((not best_assoc.isAvailable()) || (assoc.getWeight() > best_assoc.getWeight())) {
            best_assoc = assoc;
          }
        }
      }
      if (best_assoc.isAvailable()) {
        momentum = edm4hep::utils::magnitude(best_assoc.getSim().getMomentum());
      } else {
        warning("Can't find association for cluster. Skipping...");
        continue;
      }
    }

    feature_tensor.addToFloatData(momentum);
    feature_tensor.addToFloatData(cluster.getEnergy() / momentum);
    auto pos = cluster.getPosition();
    feature_tensor.addToFloatData(edm4hep::utils::anglePolar(pos));
    feature_tensor.addToFloatData(edm4hep::utils::angleAzimuthal(pos));
    for (std::size_t par_ix = 0; par_ix < cluster.shapeParameters_size(); par_ix++) {
      feature_tensor.addToFloatData(cluster.getShapeParameters(par_ix));
    }

    if (cluster_assocs != nullptr) {
      edm4eic::MCRecoClusterParticleAssociation best_assoc;
      for (auto assoc : *cluster_assocs) {
        if (assoc.getRec() == cluster) {
          if ((not best_assoc.isAvailable()) || (assoc.getWeight() > best_assoc.getWeight())) {
            best_assoc = assoc;
          }
        }
      }
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

} // namespace eicrecon
#endif
