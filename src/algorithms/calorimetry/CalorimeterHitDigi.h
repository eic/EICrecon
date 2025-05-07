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

#include <DD4hep/IDDescriptor.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/EDM4eicVersion.h>
#include <stdint.h>
#include <functional>
#include <random>
#include <string>
#include <string_view>

#include "CalorimeterHitDigiConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace edm4eic {
class MCRecoCalorimeterHitAssociationCollection;
}
namespace edm4hep {
class RawCalorimeterHitCollection;
}
namespace edm4hep {
class SimCalorimeterHit;
}
namespace edm4hep {
class SimCalorimeterHitCollection;
}

namespace eicrecon {

using CalorimeterHitDigiAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::SimCalorimeterHitCollection>,
    algorithms::Output<
#if EDM4EIC_VERSION_MAJOR >= 7
        edm4hep::RawCalorimeterHitCollection, edm4eic::MCRecoCalorimeterHitAssociationCollection
#else
        edm4hep::RawCalorimeterHitCollection
#endif
        >>;

class CalorimeterHitDigi : public CalorimeterHitDigiAlgorithm,
                           public WithPodConfig<CalorimeterHitDigiConfig> {

public:
  CalorimeterHitDigi(std::string_view name) : CalorimeterHitDigiAlgorithm {
    name, {"inputHitCollection"},
#if EDM4EIC_VERSION_MAJOR >= 7
        {"outputRawHitCollection", "outputRawHitAssociationCollection"},
#else
        {"outputRawHitCollection"},
#endif
        "Smear energy deposit, digitize within ADC range, add pedestal, "
        "convert time with smearing resolution, and sum signals."
  }
  {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  // unitless counterparts of inputs
  double stepTDC{0}, tRes{0};

  uint64_t id_mask{0};

  std::function<double(const edm4hep::SimCalorimeterHit& h)> corrMeanScale;

  dd4hep::IDDescriptor id_spec;

private:
  const algorithms::GeoSvc& m_geo = algorithms::GeoSvc::instance();

  mutable std::default_random_engine m_generator;
  mutable std::normal_distribution<double> m_gaussian;
};

} // namespace eicrecon
