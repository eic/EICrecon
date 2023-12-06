// Created by Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "services/geometry/dd4hep/DD4hep_service.h"

// Event Model related classes
#include <edm4hep/TrackerHitCollection.h>
#include <edm4eic/RawTrackerHit.h>

#include "algorithms/fardetectors/FarDetectorTrackerCluster.h"

#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"

namespace eicrecon {

class FarDetectorTrackerCluster_factory :
public JOmniFactory<FarDetectorTrackerCluster_factory,FarDetectorTrackerClusterConfig> {

private:
    eicrecon::FarDetectorTrackerCluster m_algo;

    PodioInput<edm4eic::RawTrackerHit> m_hits_input    {this};
    PodioOutput<edm4hep::TrackerHit>   m_tracks_output {this};

    Service<DD4hep_service> m_geoSvc {this};


    ParameterRef<std::string> readout     {this, "readout",     config().readout     };
    ParameterRef<std::string> moduleField {this, "moduleField", config().moduleField };
    ParameterRef<std::string> layerField  {this, "layerField",  config().layerField  };
    ParameterRef<std::string> xField      {this, "xField",      config().xField      };
    ParameterRef<std::string> yField      {this, "yField",      config().yField      };

    ParameterRef<int> n_module {this, "n_module", config().n_module };
    ParameterRef<int> n_layer  {this, "n_layer",  config().n_layer  };

    ParameterRef<double> time_limit  {this, "time_limit",  config().time_limit };


  public:
    void Configure() {
        m_algo.applyConfig(config());
        m_algo.init(m_geoSvc().detector(), m_geoSvc().converter(), logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_tracks_output() = m_algo.process(*m_hits_input());
    }
  };

} // eicrecon
