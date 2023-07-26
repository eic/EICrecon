// Created by Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once
#include <spdlog/spdlog.h>

#include <services/geometry/dd4hep/JDD4hep_service.h>

// Event Model related classes
#include <edm4eic/TrackerHit.h>
#include <edm4eic/RawTrackerHit.h>

#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>

namespace eicrecon {

  struct TrackerProtoCluster {
    int layer;
    int module;
    std::vector<edm4eic::RawTrackerHit> associatedHits;
  };

  class TrackerProtoCluster {

  public:

    TrackerProtoCluster() = default;


      /** One time initialization **/
      void init() override;

      /** Event by event processing **/
      std::vector<eicrecon::TrackerProtoCluster*> produce(const std::shared_ptr<const JEvent> &event) override;

      //----- Define constants here ------

      dd4hep::BitFieldCoder *id_dec{nullptr};
      size_t module_idx{0}, layer_idx{0}, x_idx{0}, y_idx{0};

      std::shared_ptr<JDD4hep_service> m_geoSvc;

    // Get a configuration to be changed
    eicrecon::MatrixTransferStaticConfig& getConfig() {return m_cfg;}
    
    // Sets a configuration (config is properly copyible)
    eicrecon::MatrixTransferStaticConfig& applyConfig(eicrecon::MatrixTransferStaticConfig cfg) { m_cfg = cfg; return m_cfg;}

  private:
      eicrecon::TrackerProtoClusterConfig m_cfg;
      std::shared_ptr<spdlog::logger> m_log;              /// Logger for this factory
	
  };

} // eicrecon
