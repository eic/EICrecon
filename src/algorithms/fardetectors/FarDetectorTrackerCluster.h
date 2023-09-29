// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner

#pragma once

#include <DDRec/CellIDPositionConverter.h>
// Event Model related classes
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/TrackerHitCollection.h>

#include "FarDetectorTrackerClusterConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

  class FarDetectorTrackerCluster : public WithPodConfig<FarDetectorTrackerClusterConfig>  {

  public:

    /** One time initialization **/
    void init(std::shared_ptr<const dd4hep::rec::CellIDPositionConverter>,
              const dd4hep::Detector* det,
              std::shared_ptr<spdlog::logger>);

    /** Event by event processing **/
    std::unique_ptr<edm4hep::TrackerHitCollection> produce(const edm4eic::RawTrackerHitCollection &inputhits);

  private:
      const dd4hep::Detector*         m_detector{nullptr};
      const dd4hep::BitFieldCoder*    m_id_dec{nullptr};
      std::shared_ptr<spdlog::logger> m_log;
      std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> m_cellid_converter{nullptr};

      int m_module_idx{0};
      int m_layer_idx{0};
      int m_x_idx{0};
      int m_y_idx{0};

  };

} // eicrecon
