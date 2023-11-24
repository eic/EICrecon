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

#include <DD4hep/Detector.h>
#include <algorithms/algorithm.h>
#include <algorithms/random.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <spdlog/logger.h>
#include <stdint.h>
#include <memory>
#include <random>

#include "CalorimeterHitDigiConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

  using CalorimeterHitDigiAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      edm4hep::SimCalorimeterHitCollection
    >,
    algorithms::Output<
      edm4hep::RawCalorimeterHitCollection
    >
  >;

  class CalorimeterHitDigi
  : public CalorimeterHitDigiAlgorithm,
    public WithPodConfig<CalorimeterHitDigiConfig> {

  public:
    CalorimeterHitDigi(std::string_view name)
      : CalorimeterHitDigiAlgorithm{name,
                            {"inputHitCollection"},
                            {"outputRawHitCollection"},
                            "Smear energy deposit, digitize within ADC range, add pedestal, "
                            "convert time with smearing resolution, and sum signals."} {}

    void init(const dd4hep::Detector* detector, std::shared_ptr<spdlog::logger>& logger);
    void process(const Input&, const Output&) const final;

  private:

    // unitless counterparts of inputs
    double           dyRangeADC{0}, stepTDC{0}, tRes{0};

    uint64_t         id_mask{0};

  private:
    const dd4hep::Detector* m_detector;
    std::shared_ptr<spdlog::logger> m_log;

  private:
    algorithms::Generator m_rng = algorithms::RandomSvc::instance().generator();

  };

} // namespace eicrecon
