// Created by Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "services/geometry/dd4hep/DD4hep_service.h"

// Event Model related classes
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/TrackSegment.h>
#include <algorithms/fardetectors/FarDetectorLinearTracking.h>

#include <extensions/jana/JChainMultifactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>

namespace eicrecon {

class FarDetectorLinearTracking_factory :
    public JOmniFactory<FarDetectorLinearTracking_factory, FarDetectorLinearTrackingConfig> {

    eicrecon::FarDetectorLinearTracking   m_algo;        // Actual algorithm

    PodioInput<edm4hep::TrackerHit>    m_hits_input    {this};
    PodioOutput<edm4eic::TrackSegment> m_tracks_output {this};

    Service<DD4hep_service> m_geoSvc {this};

    ParameterRef<std::string> readout     {this, "readout",     config().readout     };
    ParameterRef<std::string> moduleField {this, "moduleField", config().moduleField };
    ParameterRef<std::string> layerField  {this, "layerField",  config().layerField  };

    ParameterRef<int> n_layer         {this, "n_layer",         config().n_layer         };
    ParameterRef<int> layer_hits_max  {this, "layer_hits_max",  config().layer_hits_max  };

    ParameterRef<float> chi2_max     {this, "chi2_max",  config().chi2_max };

  public:
    void Configure() {
        m_algo.applyConfig(config());
        m_algo.init(m_geoSvc().detector(), logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_tracks_output() = m_algo.process(*m_hits_input());
    }
    };

} // eicrecon
