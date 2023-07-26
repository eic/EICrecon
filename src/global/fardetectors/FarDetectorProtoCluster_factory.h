// Created by Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once
#include <spdlog/spdlog.h>

#include <services/geometry/dd4hep/JDD4hep_service.h>

// Event Model related classes
#include <edm4eic/TrackerHit.h>
#include <edm4eic/RawTrackerHit.h>
#include <algorithms/fardetectors/TrackerProtoCluster.h>
#include <algorithms/fardetectors/TrackerProtoClusterConfig.h>

#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>

namespace eicrecon {

  class FarDetectorProtoCluster_factory : 
  public JChainFactoryT<eicrecon::TrackerProtoCluster, eicrecon::TrackerProtoClusterConfig, JFactoryT>{

  public:

    FarDetectorProtoCluster_factory(const std::vector<std::string> default_input_tags):
      JChainFactoryT(std::move(default_input_tags),cfg ) {
    }


      /** One time initialization **/
      void Init() override;

      /** On run change preparations **/
      void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

      /** Event by event processing **/
      void Process(const std::shared_ptr<const JEvent> &event) override;

      //----- Define constants here ------
      std::string m_readout{"TaggerTrackerHits"};
      std::string m_moduleField{"module"};
      std::string m_layerField{"layer"};
      std::string m_xField{"x"};
      std::string m_yField{"y"};


      std::shared_ptr<JDD4hep_service> m_geoSvc;


  private:
      std::shared_ptr<spdlog::logger> m_log;              // Logger for this factory
      eicrecon::TrackerProtoCluster   m_reco_algo;        // Actual digitisation algorithm
 
  };

} // eicrecon
