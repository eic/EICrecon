// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Chao Peng, Jihee Kim, Sylvester Joosten, Whitney Armstrong, Wouter Deconinck, David Lawrence, Derek Anderson

/*
 *  An algorithm to group readout hits from a calorimeter
 *  Energy is summed
 *
 *  Author: Chao Peng (ANL), 03/31/2021
 */

#pragma once

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DDRec/CellIDPositionConverter.h>
#include <Parsers/Primitives.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <stdint.h>
#include <cstddef>
#include <functional>
#include <gsl/pointers>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "CalorimeterHitsMergerConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

// aliases for convenience
using MergeMap = std::unordered_map<uint64_t, std::vector<std::size_t>>;
using RefField = std::pair<std::string, int>;
using MapFunc  = std::function<int(const edm4eic::CalorimeterHit&)>;

using CalorimeterHitsMergerAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::CalorimeterHitCollection>,
                          algorithms::Output<edm4eic::CalorimeterHitCollection>>;

class CalorimeterHitsMerger : public CalorimeterHitsMergerAlgorithm,
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
  uint64_t ref_mask{0};

private:
  mutable std::map<std::string, MapFunc> ref_maps;
  dd4hep::IDDescriptor id_desc;
  dd4hep::BitFieldCoder* id_decoder;

private:
  const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};
  const dd4hep::rec::CellIDPositionConverter* m_converter{
      algorithms::GeoSvc::instance().cellIDPositionConverter()};

private:
  void build_merge_map(const edm4eic::CalorimeterHitCollection* in_hits, MergeMap& merge_map) const;
};

} // namespace eicrecon
