// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang

#include "LGADHitReconstruction.h"

#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <edm4eic/Cov3f.h>
#include <edm4eic/CovDiag3f.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <iterator>
#include <spdlog/common.h>
#include <stddef.h>
#include <utility>
#include <vector>

#include "TMatrixT.h"

namespace eicrecon {

void LGADHitReconstruction::init() {

  m_converter = algorithms::GeoSvc::instance().cellIDPositionConverter();
  m_detector  = algorithms::GeoSvc::instance().detector();
  auto seg = m_detector -> readout(m_cfg.readout).segmentation();
  auto type = seg.type();
  m_decoder = seg.decoder();
}

dd4hep::rec::CellID LGADHitReconstruction::getSensorInfos(const dd4hep::rec::CellID& id) const {
  // CellID for BarrelTOF is composed of 6 parts
  // system, layer, module, sensor, x, y
  // If we fix x and y to zero, what remains will be the sensor information only
  auto id_return = id;
  m_decoder->set(id_return, "x", 0);
  m_decoder->set(id_return, "y", 0);
  return id_return;
}

void LGADHitReconstruction::process(const LGADHitReconstruction::Input& input,
		                    const LGADHitReconstruction::Output& output) const {
  using dd4hep::mm;

  const auto [TDCADC_hits] = input;
  auto [rec_hits] = output;

  // collection of ADC values from all sensors and group them by sensor 
  std::unordered_map<dd4hep::rec::CellID, std::vector<HitInfo>> hitsBySensors;

  for (const auto& TDCADC_hit : *TDCADC_hits) {

    auto id = TDCADC_hit.getCellID();

    // Get position and dimension
    auto pos = m_converter->position(id);
    // Get sensors info
    auto sensorID = this->getSensorInfos(id);
    hitsBySensors[sensorID].emplace_back(pos.x(), pos.y(), pos.z(), 
		                         int(TDCADC_hit.getCharge()),
                                         int(TDCADC_hit.getTimeStamp()), id);
  }

  for (const auto& sensor : hitsBySensors) {
    // Right now the clustering algorithm a simple average over all hits in a sensors
    // Will be problematic near the edges, but it's just an illustration
    double ave_x = 0, ave_y = 0, ave_z = 0;
    double tot_charge = 0;
    const auto& hits  = sensor.second;
    if(hits.size() == 0) continue;
    // find cellID for the cell with maximum ADC value within a sensor
    auto id         = hits[0].id;
    auto curr_adc   = hits[0].adc;
    auto first_tdc  = hits[0].tdc;

    for (const auto& hit : hits) {
      // weigh all hits by ADC value
      ave_x += hit.adc * hit.x;
      ave_y += hit.adc * hit.y;
      ave_z += hit.adc * hit.z;

      tot_charge += hit.adc;
      if (hit.adc > curr_adc) {
        curr_adc = hit.adc;
        id       = hit.id;
      }
      first_tdc = std::min(first_tdc, hit.tdc);
    }

    ave_x /= tot_charge;
    ave_y /= tot_charge;
    ave_z /= tot_charge;

    // adc to charge
    float charge = tot_charge * m_cfg.c_slope + m_cfg.c_intercept;
    // TDC to time
    float time = first_tdc * m_cfg.t_slope + m_cfg.t_intercept;

    auto cellSize = m_converter->cellDimensions(id);
    double varX = cellSize[0] / mm;
    varX *= varX;
    double varY = cellSize[1] / mm;
    varY *= varY;
    double varZ = cellSize.size() > 2? cellSize[2] / mm : 0;
    varZ *= varZ;

    rec_hits->create(id,
                     edm4hep::Vector3f{static_cast<float>(ave_x / mm),
                                       static_cast<float>(ave_y / mm),
                                       static_cast<float>(ave_z / mm)}, // mm
                     edm4eic::CovDiag3f{varX, varY, varZ}, // should be the covariance of position
                     time,                                 // ns
                     0.0F,                                 // covariance of time
                     charge,                               // total ADC sum
                     0.0F);                                // Error on the energy
  }
}

} // namespace eicrecon
