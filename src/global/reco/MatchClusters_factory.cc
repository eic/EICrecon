// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <memory>

#include <JANA/JEvent.h>

#include <spdlog/spdlog.h>

#include "MatchClusters_factory.h"

#include <edm4hep/MCParticle.h>
#include <edm4eic/Cluster.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>

#include "extensions/spdlog/SpdlogExtensions.h"


namespace eicrecon {

    void MatchClusters_factory::Init() {

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(GetApplication(), GetPrefix(), "info");

        m_match_algo.init(m_log);

        // Set up association tags
        for (const std::string& input_tag : GetInputTags()) {
            // "EcalEndcapNClusters" => "EcalEndcapNClusterAssociations"
            auto pos = input_tag.find("Clusters");

            // Validate that our input collection names conform to this convention
            if (pos == std::string::npos) {
                throw std::runtime_error(fmt::format("input_tag=\"{}\" doesn't end with \"Clusters\"", input_tag));
            }
            std::string output_tag = input_tag;
            output_tag.replace(pos, 8, "ClusterAssociations");
            m_input_assoc_tags.push_back(output_tag);
        }
    }

    void MatchClusters_factory::BeginRun(const std::shared_ptr<const JEvent> &event) {
        // Nothing to do here
    }

    void MatchClusters_factory::Process(const std::shared_ptr<const JEvent> &event) {

        // TODO make input tags changeable
        auto mc_particles = event->Get<edm4hep::MCParticle>("MCParticles");
        auto charged_particles = event->Get<edm4eic::ReconstructedParticle>("ReconstructedChargedParticles");
        auto charged_particle_assocs = event->Get<edm4eic::MCRecoParticleAssociation>("ReconstructedChargedParticleAssociations");

        using ClustersVector = std::vector<const edm4eic::Cluster*>;
        using ClustersAssocVector = std::vector<const edm4eic::MCRecoClusterParticleAssociation*>;

        std::vector<ClustersVector> input_cluster_vectors;
        std::vector<ClustersAssocVector> input_cluster_assoc;

        for(auto &input_tag: GetInputTags()) {
            auto clusters = event->Get<edm4eic::Cluster>(input_tag);
            input_cluster_vectors.push_back(clusters);

            m_log->debug("Clusters '{}' len: {}", input_tag,  clusters.size());
            for(const auto *cluster: clusters) {
                m_log->debug("  {} {}", cluster->getObjectID().collectionID, cluster->getEnergy());
            }
        }

        for(auto &input_tag: m_input_assoc_tags) {
            auto assocs = event->Get<edm4eic::MCRecoClusterParticleAssociation>(input_tag);
            input_cluster_assoc.push_back(assocs);

            m_log->debug("Associations '{}' len: {}", input_tag, assocs.size());
            for(const auto *assoc: assocs) {
                m_log->debug("  {} {} {} {}", assoc->getRec().getObjectID().index, assoc->getSim().getObjectID().index, assoc->getRec().getEnergy(), assoc->getSim().getEnergy());
            }
        }


        auto [matched_particles, matched_assocs] = m_match_algo.execute(mc_particles,
                                                                              charged_particles,
                                                                              charged_particle_assocs,
                                                                              input_cluster_vectors,
                                                                              input_cluster_assoc);

        SetCollection<edm4eic::ReconstructedParticle>(GetOutputTags()[0], std::unique_ptr<edm4eic::ReconstructedParticleCollection>(matched_particles));
        SetCollection<edm4eic::MCRecoParticleAssociation>(GetOutputTags()[1], std::unique_ptr<edm4eic::MCRecoParticleAssociationCollection>(matched_assocs));

    }
} // eicrecon
