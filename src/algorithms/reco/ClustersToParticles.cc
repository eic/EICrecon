// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ePIC Collaboration

#include <edm4eic/EDM4eicVersion.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <podio/ObjectID.h>
#include <podio/detail/Link.h>
#include <podio/detail/LinkCollectionImpl.h>
#include <cmath>
#include <memory>
#include <tuple>

#include "ClustersToParticles.h"

namespace eicrecon {

void ClustersToParticles::init() {}

void ClustersToParticles::process(const ClustersToParticles::Input& input,
                                  const ClustersToParticles::Output& output) const {
  const auto [clusters, cluster_assocs] = input;
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  auto [parts, part_links, part_assocs] = output;
#else
  auto [parts, part_assocs] = output;
#endif

  for (const auto& cluster : *clusters) {
    const auto energy = cluster.getEnergy();
    const auto pos    = cluster.getPosition();

    // Calculate momentum assuming massless particle (photon)
    const auto momentum_mag = energy;
    const auto pos_mag      = std::sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);

    edm4hep::Vector3f momentum{0, 0, 0};
    if (pos_mag > 0) {
      momentum.x = momentum_mag * pos.x / pos_mag;
      momentum.y = momentum_mag * pos.y / pos_mag;
      momentum.z = momentum_mag * pos.z / pos_mag;
    }

    debug("Converting cluster: index={:<4} energy={:<8.3f} position=({:<8.3f}, {:<8.3f}, {:<8.3f})",
          cluster.getObjectID().index, energy, pos.x, pos.y, pos.z);

    auto rec_part = parts->create();
    rec_part.addToClusters(cluster);
    rec_part.setType(0);
    rec_part.setEnergy(energy);
    rec_part.setMomentum(momentum);
    rec_part.setCharge(0.0f);     // neutral particle
    rec_part.setMass(0.0f);       // assume photon
    rec_part.setGoodnessOfPID(0); // assume no PID until proven otherwise
    rec_part.setReferencePoint({pos.x, pos.y, pos.z});

    // Handle associations if provided
    double max_weight = -1.;
    for (auto cluster_assoc : *cluster_assocs) {
      if (cluster_assoc.getRec() == cluster) {
        trace("Found cluster association: index={} -> index={}, weight={}",
              cluster_assoc.getRec().getObjectID().index,
              cluster_assoc.getSim().getObjectID().index, cluster_assoc.getWeight());
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
        auto part_link = part_links->create();
        part_link.setFrom(rec_part);
        part_link.setTo(cluster_assoc.getSim());
        part_link.setWeight(cluster_assoc.getWeight());
#endif
        auto part_assoc = part_assocs->create();
        part_assoc.setRec(rec_part);
        part_assoc.setSim(cluster_assoc.getSim());
        part_assoc.setWeight(cluster_assoc.getWeight());

        if (max_weight < cluster_assoc.getWeight()) {
          max_weight = cluster_assoc.getWeight();
          // Reference point already set above
        }
      }
    }
  }
}

} // namespace eicrecon
