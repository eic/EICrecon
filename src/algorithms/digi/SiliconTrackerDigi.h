// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov

#pragma once

#include <TRandomGen.h>
#include <algorithms/algorithm.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <spdlog/logger.h>
#include <functional>
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
      edm4eic::RawTrackerHitCollection
    >
  >;

  class SiliconTrackerDigi
  : public SiliconTrackerDigiAlgorithm,
    public WithPodConfig<SiliconTrackerDigiConfig> {

  public:
    SiliconTrackerDigi(std::string_view name)
      : SiliconTrackerDigiAlgorithm{name,
                            {"inputHitCollection"},
                            {"outputRawHitCollection"},
                            "Apply threshold, digitize within ADC range, "
                            "convert time with smearing resolution."} {}

    void init(std::shared_ptr<spdlog::logger>& logger);
    void process(const Input&, const Output&) const final;

  private:
    /** algorithm logger */
    std::shared_ptr<spdlog::logger> m_log;

    /** Random number generation*/
    TRandomMixMax m_random;
    std::function<double()> m_gauss;

    // FIXME replace with standard random engine
    //std::default_random_engine generator; // TODO: need something more appropriate here
    //std::normal_distribution<double> m_normDist; // defaults to mean=0, sigma=1

    //algorithms::Generator m_rng = algorithms::RandomSvc::instance().generator();

  };

} // eicrecon
