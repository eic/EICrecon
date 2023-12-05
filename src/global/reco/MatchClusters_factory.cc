// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <spdlog/logger.h>
#include <memory>
#include <ranges>
#include <stdexcept>

#include "MatchClusters_factory.h"


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
        size_t i = 0;
        auto mc_particles = static_cast<const edm4hep::MCParticleCollection*>(event->GetCollectionBase(GetInputTags()[i++]));
        auto charged_particles = static_cast<const edm4eic::ReconstructedParticleCollection*>(event->GetCollectionBase(GetInputTags()[i++]));
        auto charged_particle_assocs = static_cast<const edm4eic::MCRecoParticleAssociationCollection*>(event->GetCollectionBase(GetInputTags()[i++]));

        std::vector<const edm4eic::ClusterCollection*> cluster_collections;
        std::vector<const edm4eic::MCRecoClusterParticleAssociationCollection*> cluster_assoc_collections;

        // FIXME Use proper stride and adjacent views in C++23
        auto stride = [](int n) {
            return [s = -1, n](auto const&) mutable { s = (s + 1) % n; return s == 0; };
        };

        for (auto& input_tag : GetInputTags() | std::views::drop(i) | std::views::filter(stride(2))) {
            auto clusters = static_cast<const edm4eic::ClusterCollection*>(event->GetCollectionBase(input_tag));
            cluster_collections.push_back(clusters);

            m_log->debug("Clusters '{}' len: {}", input_tag,  clusters->size());
            for(const auto cluster : *clusters) {
                m_log->debug("  {} {}", cluster.getObjectID().collectionID, cluster.getEnergy());
            }
        }

        for (auto& input_tag : GetInputTags() | std::views::drop(i+1) | std::views::filter(stride(2))) {
            auto assocs = static_cast<const edm4eic::MCRecoClusterParticleAssociationCollection*>(event->GetCollectionBase(input_tag));
            cluster_assoc_collections.push_back(assocs);

            m_log->debug("Associations '{}' len: {}", input_tag, assocs->size());
            for(const auto assoc : *assocs) {
                m_log->debug("  {} {} {} {}", assoc.getRecID(), assoc.getSimID(), assoc.getRec().getEnergy(), assoc.getSim().getEnergy());
            }
        }


        auto [matched_particles, matched_assocs] = m_match_algo.execute(mc_particles,
                                                                              charged_particles,
                                                                              charged_particle_assocs,
                                                                              cluster_collections,
                                                                              cluster_assoc_collections);

        SetCollection<edm4eic::ReconstructedParticle>(GetOutputTags()[0], std::move(matched_particles));
        SetCollection<edm4eic::MCRecoParticleAssociation>(GetOutputTags()[1], std::move(matched_assocs));

    }
} // eicrecon
