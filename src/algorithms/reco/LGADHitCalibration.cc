// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang

#include "LGADHitCalibration.h"

#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <algorithms/geo.h>
#include <edm4eic/CovDiag3f.h>
#include <edm4hep/Vector3f.h>
#include <algorithm>
#include <cmath>
#include <gsl/pointers>
#include <vector>

#include "algorithms/reco/LGADHitCalibrationConfig.h"

namespace eicrecon {

void LGADHitCalibration::init() {
  m_converter = algorithms::GeoSvc::instance().cellIDPositionConverter();
}

void LGADHitCalibration::process(const LGADHitCalibration::Input& input,
                                 const LGADHitCalibration::Output& output) const {
  using dd4hep::mm;

  const auto [TDCADC_hits] = input;
  auto [calibrated_hits]   = output;

  for (const auto& TDCADC_hit : *TDCADC_hits) {

    auto id = TDCADC_hit.getCellID();

    // Get position and dimension
    dd4hep::Position pos;
    try {
      pos = m_converter->position(id);
    } catch (const std::exception& e) {
      error("Failed to get position for cell ID {:x}: {}", id, e.what());
      continue; // Skip this hit and continue with the next one
    }
    double ADC = TDCADC_hit.getCharge();
    double TDC = TDCADC_hit.getTimeStamp();

    // adc to charge
    double charge = ADC * m_cfg.c_slope + m_cfg.c_intercept;
    // TDC to time
    float time = TDC * m_cfg.t_slope + m_cfg.t_intercept;

    auto cellSize = m_converter->cellDimensions(id);
    // sqrt(12) factor convertes ranges of uniform distribution to it's standard deviation
    double varX = cellSize[0] / mm / std::sqrt(12.);
    varX *= varX; // square of cell size
    double varY = cellSize[1] / mm / std::sqrt(12.);
    varY *= varY;
    double varZ = cellSize.size() > 2 ? cellSize[2] / mm / std::sqrt(12.) : 0;
    varZ *= varZ;

    auto hit = calibrated_hits->create(
        id,
        edm4hep::Vector3f{static_cast<float>(pos.x()), static_cast<float>(pos.y()),
                          static_cast<float>(pos.z())},
        edm4eic::CovDiag3f{varX, varY, varZ}, // should be the covariance of position
        time,                                 // ns
        m_cfg.t_slope / std::sqrt(12.),       // covariance of time
        std::max(0., charge),                 // total ADC sum
        m_cfg.c_slope / std::sqrt(12.));      // Error on the energy
    hit.setRawHit(TDCADC_hit);
  }
}

} // namespace eicrecon
