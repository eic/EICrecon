// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Stephen Maple

#pragma once

#include <edm4eic/ReconstructedParticleCollection.h>
#include "edm4hep/utils/vector_utils.h"

double calc_isolation(const edm4eic::ReconstructedParticle& rcp,
		      const edm4eic::ReconstructedParticleCollection& rcparts,
		      double cone_R) {

  const edm4eic::Cluster* lead_cluster = nullptr;
  double max_energy = -1.0;
  
  for (const auto& cluster : rcp.getClusters()) {
    if (cluster.getEnergy() > max_energy) {
      max_energy = cluster.getEnergy();
      lead_cluster = &cluster;
    }
  }
  
  // If no clusters are associated with the reconstructed particle, return 0
  if (!lead_cluster) {
    return 0.0;
  }
  
  const auto& lead_pos = lead_cluster->getPosition();
  double lead_eta = edm4hep::utils::eta(lead_pos);
  double lead_phi = edm4hep::utils::angleAzimuthal(lead_pos);
  
  // Sum energy within cone centred on lead cluster
  double isolation_energy = 0.0;
  
  for (const auto& other_rcp : rcparts) {
    // Note: other_rcp will also iterate over the rcp passed to calc_isolation(), if
    // rcp is part of the rcparts collection
    for (const auto& other_cluster : other_rcp.getClusters()) {
      
      const auto& other_pos = other_cluster.getPosition();
      double other_eta = edm4hep::utils::eta(other_pos);
      double other_phi = edm4hep::utils::angleAzimuthal(other_pos);
      
      double d_eta = other_eta - lead_eta;
      double d_phi = other_phi - lead_phi;
      
      // Adjust d_phi to be in the range (-pi, pi)
      if (d_phi > M_PI) d_phi -= 2 * M_PI;
      if (d_phi < -M_PI) d_phi += 2 * M_PI;
      
      double dR = std::sqrt(std::pow(d_eta, 2) + std::pow(d_phi, 2));
      
      // Check if the cluster is within the isolation cone
      if (dR < cone_R) {
        isolation_energy += other_cluster.getEnergy();
      }
    }
  }
  
  // Calculate and return the isolation value
  if (isolation_energy > 0) {
    return lead_cluster->getEnergy() / isolation_energy;
  } else {
    return 0.0;  // Avoid division by zero
  }
}
