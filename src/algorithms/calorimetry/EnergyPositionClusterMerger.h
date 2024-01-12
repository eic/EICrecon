// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten

#pragma once

#include <limits>

#include <algorithms/algorithm.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4hep/utils/vector_utils.h>

#include "algorithms/interfaces/WithPodConfig.h"
#include "EnergyPositionClusterMergerConfig.h"

namespace eicrecon {

  using EnergyPositionClusterMergerAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      edm4eic::ClusterCollection,
      edm4eic::MCRecoClusterParticleAssociationCollection,
      edm4eic::ClusterCollection,
      edm4eic::MCRecoClusterParticleAssociationCollection
    >,
    algorithms::Output<
      edm4eic::ClusterCollection,
      edm4eic::MCRecoClusterParticleAssociationCollection
    >
  >;

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
  class EnergyPositionClusterMerger
      : public EnergyPositionClusterMergerAlgorithm,
        public WithPodConfig<EnergyPositionClusterMergerConfig> {

  public:
    EnergyPositionClusterMerger(std::string_view name)
      : EnergyPositionClusterMergerAlgorithm{name,
                            {"energyClusterCollection", "energyClusterAssociations",
                             "positionClusterCollection", "positionClusterAssociations"},
                            {"outputClusterCollection", "outputClusterAssociations"},
                            "Merge energy and position clusters if matching."} {}

  private:
    std::shared_ptr<spdlog::logger> m_log;

  public:

    void init(std::shared_ptr<spdlog::logger>& logger) {
        m_log = logger;
    }

    void process(const Input& input, const Output& output) const final {

        const auto [energy_clus, energy_assoc, pos_clus, pos_assoc] = input;
        auto [merged_clus, merged_assoc] = output;

        m_log->debug( "Merging energy and position clusters for new event" );

        if (energy_clus->size() == 0 && pos_clus->size() == 0) {
            m_log->debug( "Nothing to do for this event, returning..." );
            return;
        }

        std::vector<bool> consumed(energy_clus->size(), false);

        // use position clusters as starting point
        for (const auto& pc : *pos_clus) {

            m_log->trace(" --> Processing position cluster {}, energy: {}", pc.getObjectID().index, pc.getEnergy());

            // check if we find a good match
            int best_match    = -1;
            double best_delta = std::numeric_limits<double>::max();
            for (size_t ie = 0; ie < energy_clus->size(); ++ie) {
                if (consumed[ie]) {
                    continue;
                }

                const auto& ec = (*energy_clus)[ie];

                m_log->trace("  --> Evaluating energy cluster {}, energy: {}", ec.getObjectID().index, ec.getEnergy());

                // 1. stop if not within tolerance
                //    (make sure to handle rollover of phi properly)
                const double de_rel = std::abs((pc.getEnergy() - ec.getEnergy()) / ec.getEnergy());
                const double deta = std::abs(edm4hep::utils::eta(pc.getPosition())
                                           - edm4hep::utils::eta(ec.getPosition()));
                // check the tolerance for sin(dphi/2) to avoid the hemisphere problem and allow
                // for phi rollovers
                const double dphi = edm4hep::utils::angleAzimuthal(pc.getPosition())
                                  - edm4hep::utils::angleAzimuthal(ec.getPosition());
                const double dsphi = std::abs(sin(0.5 * dphi));
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

                const auto& ec = (*energy_clus)[best_match];

                auto new_clus  = merged_clus->create();
                new_clus.setEnergy(ec.getEnergy());
                new_clus.setEnergyError(ec.getEnergyError());
                new_clus.setTime(pc.getTime());
                new_clus.setNhits(pc.getNhits() + ec.getNhits());
                new_clus.setPosition(pc.getPosition());
                new_clus.setPositionError(pc.getPositionError());
                new_clus.addToClusters(pc);
                new_clus.addToClusters(ec);

                m_log->trace("   --> Found matching energy cluster {}, energy: {}", ec.getObjectID().index, ec.getEnergy() );
                m_log->trace("   --> Created a new combined cluster {}, energy: {}", new_clus.getObjectID().index, new_clus.getEnergy() );

                // find association from energy cluster
                auto ea = energy_assoc->begin();
                for (; ea != energy_assoc->end(); ++ea) {
                    if (ea->getRec() == ec) {
                        break;
                    }
                }
                // find association from position cluster if different
                auto pa = pos_assoc->begin();
                for (; pa != pos_assoc->end(); ++pa) {
                    if (pa->getRec() == pc) {
                        break;
                    }
                }
                if (ea != energy_assoc->end() || pa != pos_assoc->end()) {
                    // we must write an association
                    if (ea != energy_assoc->end() && pa != pos_assoc->end()) {
                        // we have two associations
                        if (pa->getSimID() == ea->getSimID()) {
                            // both associations agree on the MCParticles entry
                            auto clusterassoc = merged_assoc->create();
                            clusterassoc.setRecID(new_clus.getObjectID().index);
                            clusterassoc.setSimID(ea->getSimID());
                            clusterassoc.setWeight(1.0);
                            clusterassoc.setRec(new_clus);
                            clusterassoc.setSim(ea->getSim());
                        } else {
                            // both associations disagree on the MCParticles entry
                            m_log->debug("   --> Two associations added to {} and {}", ea->getSimID(), pa->getSimID());
                            auto clusterassoc1 = merged_assoc->create();
                            clusterassoc1.setRecID(new_clus.getObjectID().index);
                            clusterassoc1.setSimID(ea->getSimID());
                            clusterassoc1.setWeight(0.5);
                            clusterassoc1.setRec(new_clus);
                            clusterassoc1.setSim(ea->getSim());
                            auto clusterassoc2 = merged_assoc->create();
                            clusterassoc2.setRecID(new_clus.getObjectID().index);
                            clusterassoc2.setSimID(pa->getSimID());
                            clusterassoc2.setWeight(0.5);
                            clusterassoc2.setRec(new_clus);
                            clusterassoc2.setSim(pa->getSim());
                        }
                    } else if (ea != energy_assoc->end()) {
                        // no position association
                        m_log->debug("   --> Only added energy cluster association to {}", ea->getSimID());
                        auto clusterassoc = merged_assoc->create();
                        clusterassoc.setRecID(new_clus.getObjectID().index);
                        clusterassoc.setSimID(ea->getSimID());
                        clusterassoc.setWeight(1.0);
                        clusterassoc.setRec(new_clus);
                        clusterassoc.setSim(ea->getSim());
                    } else if (pa != pos_assoc->end()) {
                        // no energy association
                        m_log->debug("   --> Only added position cluster association to {}", pa->getSimID());
                        auto clusterassoc = merged_assoc->create();
                        clusterassoc.setRecID(new_clus.getObjectID().index);
                        clusterassoc.setSimID(pa->getSimID());
                        clusterassoc.setWeight(1.0);
                        clusterassoc.setRec(new_clus);
                        clusterassoc.setSim(pa->getSim());
                    }
                }

                // label our energy cluster as consumed
                consumed[best_match] = true;

                m_log->debug("  Matched position cluster {} with energy cluster {}", pc.getObjectID().index, ec.getObjectID().index);
                m_log->debug("  - Position cluster: (E: {}, phi: {}, z: {})", pc.getEnergy(),
                             edm4hep::utils::angleAzimuthal(pc.getPosition()), pc.getPosition().z);
                m_log->debug("  - Energy cluster: (E: {}, phi: {}, z: {})", ec.getEnergy(),
                             edm4hep::utils::angleAzimuthal(ec.getPosition()), ec.getPosition().z);
                m_log->debug("  ---> Merged cluster: (E: {}, phi: {}, z: {})", new_clus.getEnergy(),
                             edm4hep::utils::angleAzimuthal(new_clus.getPosition()), new_clus.getPosition().z);


            } else {

                m_log->debug("  Unmatched position cluster {}", pc.getObjectID().index);

            }

        }
    }
  };

} // namespace eicrecon
