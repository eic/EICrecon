// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Minho Kim

#pragma once

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <Parsers/Primitives.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <cstddef>
#include <gsl/pointers>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "algorithms/calorimetry/EdepToNpeConversionConfig.h"
#include "algorithms/interfaces/UniqueIDGenSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using EdepToNpeConversionAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::EventHeaderCollection, edm4hep::SimCalorimeterHitCollection>,
    algorithms::Output<edm4hep::SimCalorimeterHitCollection>>;

class EdepToNpeConversion : public EdepToNpeConversionAlgorithm,
                            public WithPodConfig<EdepToNpeConversionConfig> {

public:
  EdepToNpeConversion(std::string_view name)
      : EdepToNpeConversionAlgorithm{name, {"EventHeader", "inputHits"}, {"outputHits"}, {}} {}
  void init() final;
  void process(const Input&, const Output&) const final;

private:
  double get_edep_to_npe_factor(const edm4hep::SimCalorimeterHit& hit) const;

  const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};
  const algorithms::UniqueIDGenSvc& m_uid = algorithms::UniqueIDGenSvc::instance();

  dd4hep::IDDescriptor m_id_spec;
  const dd4hep::BitFieldCoder* m_id_dec = nullptr;
  std::vector<std::size_t> m_field_idxs{};
  std::map<std::vector<int>, double> m_edep_to_npe_lut{};
};

} // namespace eicrecon
