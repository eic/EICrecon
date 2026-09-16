// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim, Sylvester Joosten, Derek Anderson, Wouter Deconinck

#pragma once

#include <DD4hep/IDDescriptor.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4hep/CaloHitContributionCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include "SimCalorimeterHitProcessorConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using SimCalorimeterHitProcessorAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::SimCalorimeterHitCollection>,
                          algorithms::Output<edm4hep::SimCalorimeterHitCollection,
                                             edm4hep::CaloHitContributionCollection>>;

class SimCalorimeterHitProcessor : public SimCalorimeterHitProcessorAlgorithm,
                                   public WithPodConfig<SimCalorimeterHitProcessorConfig> {

public:
  SimCalorimeterHitProcessor(std::string_view name)
      : SimCalorimeterHitProcessorAlgorithm{
            name,
            {"inputHitCollection"},
            {"outputHitCollection", "outputHitContributionCollection"},
            "Regroup the hits by particle, add up the hits if"
            "they have e z-segmentation, and attenuate."} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  std::optional<uint64_t> m_hit_id_mask;
  std::optional<uint64_t> m_contribution_id_mask;

  dd4hep::IDDescriptor m_id_spec;

  const algorithms::GeoSvc& m_geo = algorithms::GeoSvc::instance();

  // a reference value for attenuation
  std::optional<double> m_attenuationReferencePosition;

private:
  // attenuation function
  double get_attenuation(double zpos) const;
};

} // namespace eicrecon
