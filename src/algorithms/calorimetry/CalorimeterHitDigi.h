// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Wouter Deconinck, Sylvester Joosten, Barak Schmookler, David Lawrence

// A general digitization for CalorimeterHit from simulation
// 1. Smear energy deposit with a/sqrt(E/GeV) + b + c/E or a/sqrt(E/GeV) (relative value)
// 2. Digitize the energy with dynamic ADC range and add pedestal (mean +- sigma)
// 3. Time conversion with smearing resolution (absolute value)
// 4. Signal is summed if the SumFields are provided
//
// Author: Chao Peng
// Date: 06/02/2021


#pragma once

#include <memory>
#include <random>

#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>

#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <spdlog/spdlog.h>

#include "algorithms/interfaces/WithPodConfig.h"
#include "CalorimeterHitDigiConfig.h"

namespace eicrecon {

  class CalorimeterHitDigi : public WithPodConfig<CalorimeterHitDigiConfig> {

  public:
    void init(const dd4hep::Detector* detector, std::shared_ptr<spdlog::logger>& logger);
    std::unique_ptr<edm4hep::RawCalorimeterHitCollection> process(const edm4hep::SimCalorimeterHitCollection &simhits) ;

  private:

    // unitless counterparts of inputs
    double           dyRangeADC{0}, stepTDC{0}, tRes{0};

    uint64_t         id_mask{0};

  private:
    const dd4hep::Detector* m_detector;
    std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> m_converter;
    std::shared_ptr<spdlog::logger> m_log;

    std::default_random_engine generator; // TODO: need something more appropriate here
    std::normal_distribution<double> m_normDist; // defaults to mean=0, sigma=1

  };

} // namespace eicrecon
