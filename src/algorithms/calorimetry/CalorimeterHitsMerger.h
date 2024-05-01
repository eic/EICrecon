// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Jihee Kim, Sylvester Joosten, Whitney Armstrong, Wouter Deconinck, David Lawrence

/*
 *  An algorithm to group readout hits from a calorimeter
 *  Energy is summed
 *
 *  Author: Chao Peng (ANL), 03/31/2021
 */

#pragma once

#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <stdint.h>
#include <gsl/pointers>
#include <string>
#include <string_view>

#include "CalorimeterHitsMergerConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

  using CalorimeterHitsMergerAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      edm4eic::CalorimeterHitCollection
    >,
    algorithms::Output<
      edm4eic::CalorimeterHitCollection
    >
  >;

  class CalorimeterHitsMerger
  : public CalorimeterHitsMergerAlgorithm,
    public WithPodConfig<CalorimeterHitsMergerConfig> {

  public:
    CalorimeterHitsMerger(std::string_view name)
      : CalorimeterHitsMergerAlgorithm{name,
                            {"inputHitCollection"},
                            {"outputHitCollection"},
                            "Group readout hits from a calorimeter."} {}

    void init() final;
    void process(const Input&, const Output&) const final;

  private:
    uint64_t id_mask{0}, ref_mask{0};

  private:
    const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};
    const dd4hep::rec::CellIDPositionConverter* m_converter{algorithms::GeoSvc::instance().cellIDPositionConverter()};

  };

} // namespace eicrecon
