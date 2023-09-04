// Created by Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once
#include <spdlog/spdlog.h>

#include <services/geometry/dd4hep/JDD4hep_service.h>

// Event Model related classes
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/TrackerHitCollection.h>

#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>

#include <services/geometry/dd4hep/JDD4hep_service.h>
#include "FarDetectorTrackerClusterConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

  class FarDetectorTrackerCluster : public WithPodConfig<FarDetectorTrackerClusterConfig>  {

  public:

    /** One time initialization **/
    void init(std::shared_ptr<const dd4hep::rec::CellIDPositionConverter>,
              dd4hep::BitFieldCoder*,
              std::shared_ptr<spdlog::logger>);

    /** Event by event processing **/
    std::unique_ptr<edm4hep::TrackerHitCollection> produce(const edm4eic::RawTrackerHitCollection &inputhits);

  private:
      std::shared_ptr<spdlog::logger> m_log;              /// Logger for this factory
      dd4hep::BitFieldCoder *m_id_dec{nullptr};
      std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> m_cellid_converter{nullptr};

  };

} // eicrecon
