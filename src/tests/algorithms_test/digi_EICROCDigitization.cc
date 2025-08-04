// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Chun Yuen Tsang, Prithwish Tribedy

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <Evaluator/DD4hepUnits.h>
#include <TMath.h>
#include <algorithms/geo.h>
#include <catch2/catch_test_macros.hpp> // for AssertionHandler, operator""_catch_sr, StringRef, REQUIRE, operator<, operator==, operator>, TEST_CASE
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/RawTimeSeriesCollection.h>
#include <spdlog/common.h> // for level_enum
#include <spdlog/logger.h> // for logger
#include <spdlog/spdlog.h> // for default_logger
#include <cmath>
#include <gsl/pointers>
#include <memory> // for allocator, unique_ptr, make_unique, shared_ptr, __shared_ptr_access
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "algorithms/digi/EICROCDigitization.h"
#include "algorithms/digi/EICROCDigitizationConfig.h"

TEST_CASE("the Silicon charge sharing algorithm runs", "[EICROCDigitization]") {
  eicrecon::EICROCDigitization algo("EICROCDigitization");

  std::shared_ptr<spdlog::logger> logger = spdlog::default_logger()->clone("EICROCDigitization");
  logger->set_level(spdlog::level::trace);

  eicrecon::EICROCDigitizationConfig cfg;

  auto detector = algorithms::GeoSvc::instance().detector();
  auto id_desc  = detector->readout("MockSiliconHits").idSpec();

  cfg.tdc_bit   = 8;
  cfg.adc_bit   = 7;
  cfg.tMax      = 10 * dd4hep::ns;
  cfg.tdc_range = pow(2, cfg.tdc_bit);
  cfg.adc_range = pow(2, cfg.adc_bit);
  cfg.t_thres   = -cfg.adc_range * 0.1;

  // check if max pulse height is linearly proportional to the initial Edep
  algo.applyConfig(cfg);
  algo.init();

  SECTION("TDC vs analytic solution scan") {
    logger->info("Begin TDC vs analytic solution scan");

    // NOLINTNEXTLINE(clang-analyzer-security.FloatLoopCounter)
    for (double time = -cfg.tMax; time <= cfg.tMax; time += cfg.tMax) {
      if (time < 0) {
        logger->info("Generation pulse at the negative time");
      } else if (time == 0) {
        logger->info("Generation pulse at the first EICROC cycle");
      } else {
        logger->info("Generation pulse at the second EICROC cycle");
      }

      // test pulse with gaussian shape
      for (double tdc_frac = 0.4; tdc_frac < 1; tdc_frac += 0.1) {
        edm4hep::RawTimeSeriesCollection time_series_coll;
        auto rawhits_coll = std::make_unique<edm4eic::RawTrackerHitCollection>();

        auto pulse = time_series_coll.create();
        auto cellID =
            id_desc.encode({{"system", 0}, {"module", 0}, {"sensor", 1}, {"x", 1}, {"y", 1}});

        pulse.setCellID(cellID);
        pulse.setCharge(1.); // placeholder
        pulse.setTime(time); // placeholder
        pulse.setInterval(1);

        int test_peak_TDC   = static_cast<int>(tdc_frac * cfg.tdc_range);
        int test_peak       = static_cast<int>(0.7 * cfg.adc_range);
        int test_peak_sigma = static_cast<int>(0.1 * cfg.tdc_range);

        for (int i = 0; i < cfg.tdc_range; ++i) {
          int ADC =
              -test_peak *
              TMath::Exp(-0.5 * pow((i - test_peak_TDC) / static_cast<double>(test_peak_sigma), 2));
          pulse.addToAdcCounts(ADC);
        }

        // calculate analytically when the pulse passes t_thres
        // ADC = amp*exp(-(TDC - mean)^2/(2sigma^2))
        // TDC = mean - (-2*sigma^2*ln(ADC/amp))^0.5
        int analytic_TDC = ceil(test_peak_TDC - sqrt(-2 * pow(test_peak_sigma, 2) *
                                                     TMath::Log(cfg.adc_range * 0.1 / test_peak)));

        // Constructing input and output as per the algorithm's expected signature
        auto input  = std::make_tuple(&time_series_coll);
        auto output = std::make_tuple(rawhits_coll.get());

        algo.process(input, output);

        REQUIRE(rawhits_coll->size() == 1);
        auto hit = (*rawhits_coll)[0];
        REQUIRE(hit.getCellID() == cellID);
        REQUIRE(hit.getCharge() == test_peak);
        if (time < 0) {
          REQUIRE(hit.getTimeStamp() == analytic_TDC - cfg.tdc_range);
        } else if (time == 0) {
          REQUIRE(hit.getTimeStamp() == analytic_TDC);
        } else {
          REQUIRE(hit.getTimeStamp() == analytic_TDC + cfg.tdc_range);
        }
      }
    }
  }

  SECTION("ADC scan") {
    logger->info("Begin ADC scan");

    // test pulse with gaussian shape
    for (double adc_frac = 0.4; adc_frac < 1; adc_frac += 0.1) {
      edm4hep::RawTimeSeriesCollection time_series_coll;
      auto rawhits_coll = std::make_unique<edm4eic::RawTrackerHitCollection>();

      auto pulse = time_series_coll.create();
      auto cellID =
          id_desc.encode({{"system", 0}, {"module", 0}, {"sensor", 1}, {"x", 1}, {"y", 1}});

      pulse.setCellID(cellID);
      pulse.setCharge(1.); // placeholder
      pulse.setTime(0.);   // placeholder
      pulse.setInterval(1);

      int test_peak_TDC   = static_cast<int>(0.5 * cfg.tdc_range);
      int test_peak       = static_cast<int>(adc_frac * cfg.adc_range);
      int test_peak_sigma = static_cast<int>(0.1 * cfg.tdc_range);

      for (int i = 0; i < cfg.tdc_range; ++i) {
        int ADC =
            -test_peak *
            TMath::Exp(-0.5 * pow((i - test_peak_TDC) / static_cast<double>(test_peak_sigma), 2));
        pulse.addToAdcCounts(ADC);
      }

      // Constructing input and output as per the algorithm's expected signature
      auto input  = std::make_tuple(&time_series_coll);
      auto output = std::make_tuple(rawhits_coll.get());

      algo.process(input, output);

      REQUIRE(rawhits_coll->size() == 1);
      auto hit = (*rawhits_coll)[0];
      REQUIRE(hit.getCellID() == cellID);
      REQUIRE(hit.getCharge() == test_peak);
    }
  }
}
