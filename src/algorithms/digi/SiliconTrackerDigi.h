// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov

#pragma once

#include <TRandomGen.h>
#include <algorithms/algorithm.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <functional>
#include <string>
#include <string_view>

#include "SiliconTrackerDigiConfig.h"
#include "algorithms/interfaces/UniqueIDGenSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using SiliconTrackerDigiAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::EventHeaderCollection, edm4hep::SimTrackerHitCollection>,
    algorithms::Output<edm4eic::RawTrackerHitCollection,
                       edm4eic::MCRecoTrackerHitAssociationCollection>>;

class SiliconTrackerDigi : public SiliconTrackerDigiAlgorithm,
                           public WithPodConfig<SiliconTrackerDigiConfig> {

public:
  SiliconTrackerDigi(std::string_view name)
      : SiliconTrackerDigiAlgorithm{name,
                                    {"eventHeaderCollection", "inputHitCollection"},
                                    {"outputRawHitCollection", "outputHitAssociations"},
                                    "Apply threshold, digitize within ADC range, "
                                    "convert time with smearing resolution."} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const algorithms::UniqueIDGenSvc& m_uid = algorithms::UniqueIDGenSvc::instance();

  /** Random number generation*/
  mutable TRandomMixMax m_random;
  std::function<double()> m_gauss;

  // FIXME replace with standard random engine
  // std::default_random_engine generator; // TODO: need something more appropriate here
  // std::normal_distribution<double> m_normDist; // defaults to mean=0, sigma=1

  // algorithms::Generator m_rng = algorithms::RandomSvc::instance().generator();
};

} // namespace eicrecon
