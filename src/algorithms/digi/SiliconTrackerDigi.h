// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov

#pragma once

#include <algorithms/algorithm.h>
#include <algorithms/random.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <string>
#include <string_view>

#include "SiliconTrackerDigiConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

  using SiliconTrackerDigiAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      edm4hep::SimTrackerHitCollection
    >,
    algorithms::Output<
      edm4eic::RawTrackerHitCollection,
      edm4eic::MCRecoTrackerHitAssociationCollection
    >
  >;

  class SiliconTrackerDigi
  : public SiliconTrackerDigiAlgorithm,
    public WithPodConfig<SiliconTrackerDigiConfig> {

  public:
    SiliconTrackerDigi(std::string_view name)
      : SiliconTrackerDigiAlgorithm{name,
                            {"inputHitCollection"},
                            {"outputRawHitCollection","outputHitAssociations"},
                            "Apply threshold, digitize within ADC range, "
                            "convert time with smearing resolution."} {}

    void init(std::shared_ptr<spdlog::logger>& logger);
    void process(const Input&, const Output&) const final;

  private:
    /** algorithm logger */
    std::shared_ptr<spdlog::logger> m_log;

    /** Random number generation*/
    algorithms::Generator m_rng = algorithms::RandomSvc::instance().generator();

  };

} // eicrecon
