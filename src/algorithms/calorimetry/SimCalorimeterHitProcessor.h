// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim, Sylvester Joosten, Derek Anderson

#pragma once

#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <DD4hep/IDDescriptor.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/CaloHitContributionCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <stdint.h>
#include <string>
#include <string_view>
#include <functional>

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
            "they have the same z-segmentation, and attenuate."} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  uint64_t m_id_mask{0};

  dd4hep::IDDescriptor m_id_spec;

  const algorithms::GeoSvc& m_geo = algorithms::GeoSvc::instance();

  // a reference value for attenuation
  double m_attenuationMode;

  // flags for merge and attenuation
  bool is_merge       = false;
  bool is_attenuation = false;

private:
  edm4hep::MCParticle get_primary(const edm4hep::CaloHitContribution& contrib) const;

  // attenuation function
  double get_attenuation(double zpos) const;
};

} // namespace eicrecon
