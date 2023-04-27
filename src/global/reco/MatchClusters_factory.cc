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

#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include <algorithms/tracking/ParticlesFromTrackFitResult.h>


namespace eicrecon {

    void MatchClusters_factory::Init() {

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(GetPrefix(), "info");

        m_match_algo.init(m_log);
    }

    void MatchClusters_factory::BeginRun(const std::shared_ptr<const JEvent> &event) {
        // Nothing to do here
    }

    void MatchClusters_factory::Process(const std::shared_ptr<const JEvent> &event) {
        using ClustersVector = std::vector<const edm4eic::Cluster*>;
        using ClustersAssocVector = std::vector<const edm4eic::MCRecoClusterParticleAssociation*>;

        m_log->debug("------- Process start -------");

        // TODO make input tags changable
        auto mc_particles = event->Get<edm4hep::MCParticle>("MCParticles");

        auto charged_particles = static_cast<const edm4eic::ReconstructedParticleCollection*>(event->GetCollectionBase("ReconstructedChargedParticles"));
        auto charged_particle_assocs = static_cast<const edm4eic::MCRecoParticleAssociationCollection*>(event->GetCollectionBase("ReconstructedChargedParticlesAssociations"));
        // TODO: NWB: Switch to GetCollection<T> once JANA v2.1.1 is in

        // TODO: NWB: tracking_data is going away. Also it is redundant with charged_particles, charged_particle_assocs.
        // auto tracking_data = event->GetSingle<eicrecon::ParticlesWithAssociation>("ChargedParticlesWithAssociations");
        std::vector<ClustersVector> input_cluster_vectors;//{"OutputClusters", Gaudi::DataHandle::Writer, this};
        std::vector<ClustersAssocVector> input_cluster_assoc;//{"OutputAssociations", Gaudi::DataHandle::Writer, this};

        for(auto &input_tag: GetInputTags()) {
            auto clusters = event->Get<edm4eic::Cluster>(input_tag);
            input_cluster_vectors.push_back(clusters);
//            m_log->debug("Clusters '{}' len: {}", input_tag,  clusters.size());
//            for(auto cluster: clusters) {
//                m_log->debug("  {} {}", cluster->getObjectID().collectionID, cluster->getEnergy());
//            }
        }

        for(auto &input_tag: m_input_assoc_tags) {
            auto assocs = event->Get<edm4eic::MCRecoClusterParticleAssociation>(input_tag);
            input_cluster_assoc.push_back(assocs);

//            m_log->debug("Associations '{}' len: {}", input_tag, assocs.size());
//            for(auto assoc: assocs) {
//                m_log->debug("  {} {} {} {}", assoc->getRecID(), assoc->getSimID(), assoc->getRec().getEnergy(), assoc->getSim().getEnergy());
//            }
        }




//        auto result = m_match_algo.execute(mc_particles,
//                                           charged_particles,
//                                           charged_particle_assocs,
//                                           input_cluster_vectors,
//                                           input_cluster_assoc);

        //std::vector<edm4eic::ReconstructedParticle*> result;
//        for(size_t i=0; i < tracking_data->particles()->size(); i++) {
//            auto particle = (*tracking_data->particles())[i];
//            result.push_back(new edm4eic::ReconstructedParticle(particle));
//        }

        edm4eic::ReconstructedParticleCollection reconstructed_particles;
        reconstructed_particles.setSubsetCollection(true);
        for (auto part : *charged_particles) {
            reconstructed_particles.push_back(part);
        }

        edm4eic::MCRecoParticleAssociationCollection reconstructed_particle_assocs;
        reconstructed_particle_assocs.setSubsetCollection(true);
        for (auto assoc : *charged_particle_assocs) {
            reconstructed_particle_assocs.push_back(assoc);
        }

        SetCollection<edm4eic::ReconstructedParticle>(GetOutputTags()[0], std::move(reconstructed_particles));
        SetCollection<edm4eic::MCRecoParticleAssociation>(GetOutputTags()[1], std::move(reconstructed_particle_assocs));
    }
} // eicrecon
