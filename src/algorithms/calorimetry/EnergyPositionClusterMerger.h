// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten

#pragma once

#include <limits>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

// Event Model related classes
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/vector_utils.h>

#include "algorithms/interfaces/WithPodConfig.h"
#include "EnergyPositionClusterMergerConfig.h"

namespace eicrecon {

/** Simple algorithm to merge the energy measurement from cluster1 with the position
 * measurement of cluster2 (in case matching clusters are found). If not, it will
 * propagate the raw cluster from cluster1 or cluster2
 *
 * Matching occurs based on the cluster phi, eta and E variables, with tolerances
 * defined in the options file. A negative tolerance effectively disables
 * a check. The energy tolerance is defined as a relative number (e.g. 0.1)
 *
 * In case of ambiguity the closest cluster is merged.
 *
 * \ingroup reco
 */
  class EnergyPositionClusterMerger : public WithPodConfig<EnergyPositionClusterMergerConfig> {

  using ClustersWithAssociations = std::tuple<
        std::unique_ptr<edm4eic::ClusterCollection>,
        std::unique_ptr<edm4eic::MCRecoClusterParticleAssociationCollection>
  >;

  protected:
    std::shared_ptr<spdlog::logger> m_log;

  public:

    void init(std::shared_ptr<spdlog::logger>& logger) {
        m_log = logger;
    }

    ClustersWithAssociations process(
        const edm4eic::ClusterCollection& energy_clus,
        const edm4eic::MCRecoClusterParticleAssociationCollection& energy_assoc,
        const edm4eic::ClusterCollection& pos_clus,
        const edm4eic::MCRecoClusterParticleAssociationCollection& pos_assoc
    ) {

        m_log->debug( "Merging energy and position clusters for new event" );

        // output
        auto merged_clus = std::make_unique<edm4eic::ClusterCollection>();
        auto merged_assoc = std::make_unique<edm4eic::MCRecoClusterParticleAssociationCollection>();

        if (energy_clus.size() == 0 && pos_clus.size() == 0) {
            m_log->debug( "Nothing to do for this event, returning..." );
            return std::make_tuple(std::move(merged_clus), std::move(merged_assoc));;
        }

	std::vector<bool> consumed(energy_clus.size(), false);

        // use position clusters as starting point
        for (const auto& pc : pos_clus) {
            // check if we find a good match
            int best_match    = -1;
            double best_delta = std::numeric_limits<double>::max();
            for (size_t ie = 0; ie < energy_clus.size(); ++ie) {
                if (consumed[ie]) {
                    continue;
                }
                const auto& ec = energy_clus[ie];
                // 1. stop if not within tolerance
                //    (make sure to handle rollover of phi properly)
                const double de_rel = std::abs((pc.getEnergy() - ec.getEnergy()) / ec.getEnergy());
                // check the tolerance for sin(dphi/2) to avoid the hemisphere problem and allow
                // for phi rollovers
                const double dsphi = std::abs(sin(0.5 * (edm4eic::angleAzimuthal(pc.getPosition()) - edm4eic::angleAzimuthal(ec.getPosition()))));
                const double deta = edm4eic::eta(pc.getPosition()) - edm4eic::eta(ec.getPosition());
                if ((m_cfg.energyRelTolerance > 0 && de_rel > m_cfg.energyRelTolerance) ||
                    (m_cfg.etaTolerance > 0 && deta > m_cfg.etaTolerance) ||
                    (m_cfg.phiTolerance > 0 && dsphi > sin(0.5 * m_cfg.phiTolerance))) {
                    continue;
                }
                // --> if we get here, we have a match within tolerance. Now treat the case
                //     where we have multiple matches. In this case take the one with the closest
                //     energies.
                // 2. best match?
                const double delta = fabs(pc.getEnergy() - ec.getEnergy());
                if (delta < best_delta) {
                    best_delta = delta;
                    best_match = ie;
                }
            }
            // Create a merged cluster if we find a good match
            if (best_match >= 0) {
                const auto& ec = energy_clus[best_match];

                auto new_clus  = merged_clus->create();
                new_clus.setEnergy(ec.getEnergy());
                new_clus.setEnergyError(ec.getEnergyError());
                new_clus.setTime(pc.getTime());
                new_clus.setNhits(pc.getNhits() + ec.getNhits());
                new_clus.setPosition(pc.getPosition());
                new_clus.setPositionError(pc.getPositionError());
                new_clus.addToClusters(pc);
                new_clus.addToClusters(ec);

                // add association from energy cluster
                auto ea = energy_assoc.begin();
                for (; ea != energy_assoc.end(); ++ea) {
                    if (ea->getRec() == ec) {
                        break;
                    }
                }
                if (ea != energy_assoc.end()) {
                    auto clusterassoc = merged_assoc->create();
                    clusterassoc.setRecID(new_clus.getObjectID().index);
                    clusterassoc.setSimID(ea->getSimID());
                    clusterassoc.setWeight(1.0);
                    clusterassoc.setRec(new_clus);
                    clusterassoc.setSim(ea->getSim());
                }
                // add association from position cluster if different
                auto pa = pos_assoc.begin();
                for (; pa != pos_assoc.end(); ++pa) {
                    if (pa->getRec() == pc) {
                        break;
                    }
                }
                if (pa != pos_assoc.end() && pa->getSimID() != ea->getSimID()) {
                    auto clusterassoc = merged_assoc->create();
                    clusterassoc.setRecID(new_clus.getObjectID().index);
                    clusterassoc.setSimID(pa->getSimID());
                    clusterassoc.setWeight(1.0);
                    clusterassoc.setRec(new_clus);
                    clusterassoc.setSim(pa->getSim());
                }

                // label our energy cluster as consumed
                consumed[best_match] = true;

                m_log->debug("Matched position cluster {} with energy cluster {}\n", pc.getObjectID().index, ec.getObjectID().index);
                m_log->debug("  - Position cluster: (E: {}, phi: {}, z: {})", pc.getEnergy(),
                             edm4eic::angleAzimuthal(pc.getPosition()), pc.getPosition().z);
                m_log->debug("  - Energy cluster: (E: {}, phi: {}, z: {})", ec.getEnergy(),
                             edm4eic::angleAzimuthal(ec.getPosition()), ec.getPosition().z);
                m_log->debug("  ---> Merged cluster: (E: {}, phi: {}, z: {})", new_clus.getEnergy(),
                             edm4eic::angleAzimuthal(new_clus.getPosition()), new_clus.getPosition().z);
            }

        }

        return std::make_tuple(std::move(merged_clus), std::move(merged_assoc));
    }
  };

} // namespace eicrecon
