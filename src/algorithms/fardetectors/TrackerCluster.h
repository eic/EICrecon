// Created by Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once
#include <spdlog/spdlog.h>

#include <services/geometry/dd4hep/JDD4hep_service.h>

// Event Model related classes
#include <edm4eic/RawTrackerHit.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/TrackerHitCollection.h>

#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>

#include <services/geometry/dd4hep/JDD4hep_service.h>
#include "FarTrackerClusterConfig.h"

namespace eicrecon {

  class TrackerClusterGen {

  public:

    TrackerClusterGen() = default;

    /** One time initialization **/
    void init();
    
    /** Event by event processing **/
    std::unique_ptr<edm4hep::TrackerHitCollection> produce(const edm4eic::RawTrackerHitCollection &inputhits);
    
    // Get bit encoder
    dd4hep::BitFieldCoder* getEncoder() {return m_id_dec;}
    
    // Set bit encoder
    void setEncoder(dd4hep::BitFieldCoder *id_dec) {m_id_dec=id_dec;}

    // Get position convertor
    std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> getGeoConverter() {return m_cellid_converter;}
    //    dd4hep::rec::CellIDPositionConverter* getGeoConverter() {return m_cellid_converter;}

    // Set position convertor
    void setGeoConverter(std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> id_conv) {m_cellid_converter=id_conv;}

    // Get a configuration to be changed
    eicrecon::FarTrackerClusterConfig& getConfig() {return m_cfg;}
    
    // Sets a configuration (config is properly copyible)
    eicrecon::FarTrackerClusterConfig& applyConfig(eicrecon::FarTrackerClusterConfig cfg) { m_cfg = cfg; return m_cfg;}

  private:
      eicrecon::FarTrackerClusterConfig m_cfg;
      std::shared_ptr<spdlog::logger> m_log;              /// Logger for this factory
	
      dd4hep::BitFieldCoder *m_id_dec{nullptr};
      std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> m_cellid_converter{nullptr};
  
  };

} // eicrecon
