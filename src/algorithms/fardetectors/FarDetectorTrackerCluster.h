// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner

#pragma once

#include <algorithms/algorithm.h>
#include <DDRec/CellIDPositionConverter.h>
// Event Model related classes
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/TrackerHitCollection.h>

#include <string_view>

#include "FarDetectorTrackerClusterConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

  using FarDetectorTrackerClusterAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      std::vector<edm4eic::RawTrackerHitCollection>
    >,
    algorithms::Output<
      std::vector<edm4hep::TrackerHitCollection>
    >
  >;

  class FarDetectorTrackerCluster
  : public FarDetectorTrackerClusterAlgorithm,
    public WithPodConfig<FarDetectorTrackerClusterConfig>  {

  public:
    FarDetectorTrackerCluster(std::string_view name)
      : FarDetectorTrackerClusterAlgorithm{name,
                            {"inputHitCollection"},
                            {"outputClusterPositionCollection"},
                            "Simple weighted clustering of hits by x-y component of single detector element segmentation"} {}

    /** One time initialization **/
    void init(const dd4hep::rec::CellIDPositionConverter* converter,
              const dd4hep::Detector* det,
              std::shared_ptr<spdlog::logger>);

    /** Event by event processing **/
    void process(const Input&, const Output&) const final;

  private:
      const dd4hep::Detector*         m_detector{nullptr};
      const dd4hep::BitFieldCoder*    m_id_dec{nullptr};
      std::shared_ptr<spdlog::logger> m_log;
      const dd4hep::rec::CellIDPositionConverter* m_cellid_converter{nullptr};

      int m_x_idx{0};
      int m_y_idx{0};

  };

} // eicrecon
