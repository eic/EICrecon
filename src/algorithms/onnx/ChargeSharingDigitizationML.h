// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#pragma once

#include <DD4hep/Detector.h>
#include <DD4hep/Segmentations.h>
#include <DDRec/CellIDPositionConverter.h>

#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <cstdint>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/Vector2f.h>
#include <onnxruntime_cxx_api.h>
#include <string>
#include <string_view>
#include <vector>
#include <Eigen/Core>


#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/onnx/ChargeSharingDigitizationMLConfig.h"

namespace eicrecon {

using ChargeSharingDigitizationMLAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::SimTrackerHitCollection>,
                          algorithms::Output<edm4eic::RawTrackerHitCollection,
                                             edm4eic::MCRecoTrackerHitAssociationCollection>>;

class ChargeSharingDigitizationML : public ChargeSharingDigitizationMLAlgorithm,
                              public WithPodConfig<ChargeSharingDigitizationMLConfig> {

public:
  ChargeSharingDigitizationML(std::string_view name)
      : ChargeSharingDigitizationMLAlgorithm{name,
                                       {"SimulationHits"},
                                       {"DigitizedHits", "MCRecoAssociations"},
                                       "Determine inclusive kinematics using combined ML method."} {
  }
    
  ~ChargeSharingDigitizationML() {
    m_session->release();
  }


  void init() final;
  void process(const Input&, const Output&) const final;
  Eigen::Vector2d getSubcellPosition(const edm4hep::Vector3d global_pos, uint64_t cellID) const;
  Eigen::Vector2d getSubcellMomentum(const edm4hep::Vector3f global_mom, uint64_t cellID) const;
  Eigen::Matrix2d getTransformationMatrix(const Eigen::Vector2d point) const;

private:
  std::unique_ptr<Ort::Session> m_session;

  std::vector<std::string> m_input_names;
  std::vector<const char*> m_input_names_char;
  std::vector<std::vector<std::int64_t>> m_input_shapes;

  std::vector<std::string> m_output_names;
  std::vector<const char*> m_output_names_char;
  std::vector<std::vector<std::int64_t>> m_output_shapes;

  const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};
  const dd4hep::BitFieldCoder* m_id_dec{nullptr};
  const dd4hep::rec::CellIDPositionConverter* m_cellid_converter{algorithms::GeoSvc::instance().cellIDPositionConverter()};
  dd4hep::Segmentation m_seg;

  int m_x_idx{0};
  int m_y_idx{0};
  int m_sensor_idx{0};
  int m_module_idx{0};
  int m_layer_idx{0};
  int m_detector_idx{0};
};

} // namespace eicrecon
