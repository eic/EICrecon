// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <memory>

#include <JANA/JEvent.h>

#include <spdlog/spdlog.h>

#include "MatchClusters_factory.h"

#include <edm4eic/Cluster.h>
#include <edm4eic/MCRecoClusterParticleAssociation.h>
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "ParticlesFromTrackFitResult.h"
#include "algorithms/reco/ParticlesWithAssociation.h"

namespace eicrecon {



    void MatchClusters_factory::Init() {

        // This prefix will be used for parameters
        std::string param_prefix = "reco:" + GetTag();

        // Set input data tags properly
        InitDataTags(param_prefix);

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(param_prefix, "info");
    }

    void MatchClusters_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
        // Nothing to do here
    }

    void MatchClusters_factory::Process(const std::shared_ptr<const JEvent> &event) {
        using ClustersVector = std::vector<const edm4eic::Cluster*>;
        using ClustersAssocVector = std::vector<const edm4eic::MCRecoClusterParticleAssociation*>;
        auto tracking_data = event->GetSingle<eicrecon::ParticlesWithAssociation>("ChargedParticlesWithAssociations");
        std::vector<ClustersVector> input_cluster_vectors;//{"OutputClusters", Gaudi::DataHandle::Writer, this};
        std::vector<ClustersAssocVector> input_cluster_assoc;//{"OutputAssociations", Gaudi::DataHandle::Writer, this};

        for(auto &input_tag: GetInputTags()) {
            input_cluster_vectors.push_back(event->Get<edm4eic::Cluster>(input_tag));
        }

        for(auto &input_tag: m_input_assoc_tags) {
            auto assoc = event->Get<edm4eic::MCRecoClusterParticleAssociation>(input_tag);
            input_cluster_assoc.push_back(assoc);
        }

        std::vector<edm4eic::ReconstructedParticle*> result;
//        for(size_t i=0; i < tracking_data->particles()->size(); i++) {
//            auto particle = (*tracking_data->particles())[i];
//            result.push_back(new edm4eic::ReconstructedParticle(particle));
//        }
        Set(result);
    }
} // eicrecon