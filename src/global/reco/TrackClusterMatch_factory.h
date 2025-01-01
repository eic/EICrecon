#pragma once

#include <algorithms/logger.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>

#include "algorithms/reco/TrackClusterMatch.h"
#include "algorithms/reco/TrackClusterMatchConfig.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {
    class TrackClusterMatch_factory : public JOmniFactory<TrackClusterMatch_factory> {
    private:
        // underlying algorithm
        std::unique_ptr<eicrecon::TrackClusterMatch> m_algo;

        // inputs
        PodioInput<edm4eic::Cluster> m_cluster_input {this};
        PodioInput<edm4eic::TrackSegment> m_tracksegment_input {this};

        // output
        PodioOutput<edm4eic::ReconstructedParticle> m_reconstructedparticle_output {this};

    public: 
        void Configure() {
            m_algo = std::make_unique<eicrecon::TrackClusterMatch>(GetPrefix());
            m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
            m_algo->init();
        }

        void ChangeRun(int64_t run_number) {}
        void Process(int64_t run_number, uint64_t event_number) {
            m_algo->process({m_cluster_input(), m_tracksegment_input()},
                            {m_reconstructedparticle_output().get()});
        }

    };
}