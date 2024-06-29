// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Simon Gardner

#include "services/geometry/dd4hep/DD4hep_service.h"

// Event Model related classes
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/TrackSegment.h>
#include <algorithms/fardetectors/FarDetectorLinearTracking.h>

#include <extensions/jana/JOmniFactory.h>
#include <spdlog/logger.h>

namespace eicrecon {

class FarDetectorLinearTracking_factory :
    public JOmniFactory<FarDetectorLinearTracking_factory, FarDetectorLinearTrackingConfig> {

public:
    using AlgoT = eicrecon::FarDetectorLinearTracking;
private:
    std::unique_ptr<AlgoT> m_algo;

    VariadicPodioInput<edm4hep::TrackerHit> m_hits_input    {this};
    PodioOutput<edm4eic::TrackSegment>      m_tracks_output {this};

    ParameterRef<int>   n_layer        {this, "numLayers",       config().n_layer         };
    ParameterRef<int>   layer_hits_max {this, "layerHitsMax",    config().layer_hits_max  };
    ParameterRef<float> chi2_max       {this, "chi2Max",         config().chi2_max        };

  public:
    void Configure() {
        m_algo = std::make_unique<AlgoT>(GetPrefix());
        m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
        m_algo->applyConfig(config());
        m_algo->init();
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {

        try {
            std::vector<gsl::not_null<const edm4hep::TrackerHitCollection*>>  hits;
            for( const auto& hit : m_hits_input() ) {
                hits.push_back(gsl::not_null<const edm4hep::TrackerHitCollection*>{hit});
            }

            m_algo->process(hits, {m_tracks_output().get()});
        }
        catch(std::exception &e) {
            throw JException(e.what());
        }

    }
    };

} // eicrecon
