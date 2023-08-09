// Created by Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once
#include <spdlog/spdlog.h>

#include <services/geometry/dd4hep/JDD4hep_service.h>

// Event Model related classes
#include <edm4eic/RawTrackerHit.h>
#include <edm4eic/RawTrackerHitCollection.h>

#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>

#include <services/geometry/dd4hep/JDD4hep_service.h>
#include "TrackerClusterConfig.h"

namespace eicrecon {

  struct TrackerProtoCluster {
    int layer;
    int module;
    //edm4eic::RawTrackerHitCollection associatedHits;
    std::vector<edm4eic::RawTrackerHit> associatedHits;
  };

  class TrackerProtoClusterGen {

  public:

    TrackerProtoClusterGen() = default;


      /** One time initialization **/
      void init();

      /** Event by event processing **/
      std::vector<eicrecon::TrackerProtoCluster*> produce(const edm4eic::RawTrackerHitCollection &inputhits);

      //----- Define constants here ------

    // Get bit encoder
    dd4hep::BitFieldCoder* getEncoder() {return m_id_dec;}

    // Set bit encoder
    void setEncoder(dd4hep::BitFieldCoder *id_dec) {m_id_dec=id_dec;}

    // Get a configuration to be changed
    eicrecon::TrackerClusterConfig& getConfig() {return m_cfg;}
    
    // Sets a configuration (config is properly copyible)
    eicrecon::TrackerClusterConfig& applyConfig(eicrecon::TrackerClusterConfig cfg) { m_cfg = cfg; return m_cfg;}

  private:
      eicrecon::TrackerClusterConfig m_cfg;
      std::shared_ptr<spdlog::logger> m_log;              /// Logger for this factory
	
      dd4hep::BitFieldCoder *m_id_dec{nullptr};
  };

} // eicrecon
