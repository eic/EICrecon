// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Dmitry Kalinkin

#include <edm4hep/MCParticle.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/format.h>
#include <podio/LinkNavigator.h>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <tuple>
#include <vector>

#include "CalorimeterParticleIDPreML.h"

namespace eicrecon {

void CalorimeterParticleIDPreML::init() {
  // Nothing
}

void CalorimeterParticleIDPreML::process(const CalorimeterParticleIDPreML::Input& input,
                                         const CalorimeterParticleIDPreML::Output& output) const {

  const auto [clusters, cluster_links]   = input;
  auto [feature_tensors, target_tensors] = output;

  if (cluster_links == nullptr || cluster_links->empty()) {
    error("Cluster links are required for CalorimeterParticleIDPreML");
    throw std::runtime_error("Missing inputClusterLinks");
  }

  edm4eic::MutableTensor feature_tensor = feature_tensors->create();
  feature_tensor.addToShape(clusters->size());
  feature_tensor.addToShape(11);    // p, E/p, azimuthal, polar, 7 shape parameters
  feature_tensor.setElementType(1); // 1 - float

  edm4eic::MutableTensor target_tensor;
  target_tensor = target_tensors->create();
  target_tensor.addToShape(clusters->size());
  target_tensor.addToShape(2);     // is electron, is hadron
  target_tensor.setElementType(7); // 7 - int64

  podio::LinkNavigator<edm4eic::MCRecoClusterParticleLinkCollection> link_nav(*cluster_links);

  for (edm4eic::Cluster cluster : *clusters) {
    // FIXME: use track momentum once matching to tracks becomes available
    edm4hep::MCParticle best_sim;
    float best_weight = std::numeric_limits<float>::lowest();
    bool found_assoc  = false;
    for (const auto& [sim_particle, weight] : link_nav.getLinked(cluster)) {
      if (!found_assoc || weight > best_weight) {
        best_sim    = sim_particle;
        best_weight = weight;
        found_assoc = true;
      }
    }
    if (!found_assoc) {
      warning("Can't find association for cluster. Skipping...");
      continue;
    }
    const double momentum = edm4hep::utils::magnitude(best_sim.getMomentum());

    feature_tensor.addToFloatData(momentum);
    feature_tensor.addToFloatData(cluster.getEnergy() / momentum);
    auto pos = cluster.getPosition();
    feature_tensor.addToFloatData(edm4hep::utils::anglePolar(pos));
    feature_tensor.addToFloatData(edm4hep::utils::angleAzimuthal(pos));
    for (std::size_t par_ix = 0; par_ix < cluster.shapeParameters_size(); par_ix++) {
      feature_tensor.addToFloatData(cluster.getShapeParameters(par_ix));
    }

    int64_t is_electron = static_cast<int64_t>(best_sim.getPDG() == 11);
    int64_t is_pion     = static_cast<int64_t>(best_sim.getPDG() != 11);
    target_tensor.addToInt64Data(is_pion);
    target_tensor.addToInt64Data(is_electron);
  }

  std::size_t expected_num_entries = feature_tensor.getShape(0) * feature_tensor.getShape(1);
  if (feature_tensor.floatData_size() != expected_num_entries) {
    error("Inconsistent output tensor shape and element count: {} != {}",
          feature_tensor.floatData_size(), expected_num_entries);
    throw std::runtime_error(
        std::format("Inconsistent output tensor shape and element count: {} != {}",
                    feature_tensor.floatData_size(), expected_num_entries));
  }
}

} // namespace eicrecon
