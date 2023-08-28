// Created by Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once
#include <spdlog/spdlog.h>

#include <services/geometry/dd4hep/JDD4hep_service.h>

// Event Model related classes
#include <edm4hep/TrackerHitCollection.h>
#include <edm4eic/RawTrackerHit.h>
#include <algorithms/fardetectors/TrackerCluster.h>
#include <algorithms/fardetectors/FarTrackerClusterConfig.h>

//#include <extensions/jana/JChainMultifactoryT.h>
#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>

namespace eicrecon {

  class FarDetectorCluster_factory : 
  public JChainFactoryT<edm4hep::TrackerHit, FarTrackerClusterConfig>{

  public:

    explicit FarDetectorCluster_factory(const std::vector<std::string> default_input_tags, FarTrackerClusterConfig cfg):
      JChainFactoryT(std::move(default_input_tags),cfg ) {
    }
      
      
      /** One time initialization **/
      void Init() override;
      
      /** On run change preparations **/
      void ChangeRun(const std::shared_ptr<const JEvent> &event) override;
      
      /** Event by event processing **/
      void Process(const std::shared_ptr<const JEvent> &event) override;
      

  private:
      std::shared_ptr<spdlog::logger> m_log;              // Logger for this factory
      eicrecon::TrackerClusterGen     m_reco_algo;        // Actual digitisation algorithm
      
      dd4hep::BitFieldCoder *id_dec{nullptr};
      std::shared_ptr<JDD4hep_service> m_geoSvc;

  };

} // eicrecon
