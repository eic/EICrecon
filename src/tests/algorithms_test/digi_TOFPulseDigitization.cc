// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Chun Yuen Tsang, Prithwish Tribedy

#include <catch2/catch_test_macros.hpp> // for AssertionHandler, operator""_catch_sr, StringRef, REQUIRE, operator<, operator==, operator>, TEST_CASE
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/Vector2i.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <math.h>
#include <memory> // for allocator, unique_ptr, make_unique, shared_ptr, __shared_ptr_access
#include <spdlog/common.h> // for level_enum
#include <spdlog/logger.h> // for logger
#include <spdlog/spdlog.h> // for default_logger
#include <tuple>
#include <vector>

#include "algorithms/digi/TOFHitDigiConfig.h"
#include "algorithms/digi/TOFPulseDigitization.h"
#include <DD4hep/Segmentations.h>
#include <DDRec/CellIDPositionConverter.h>
#include <algorithms/geo.h>
#include <algorithms/logger.h>
#include <edm4eic/RawTrackerHitCollection.h>

#include "TF1.h"
#include "TGraphErrors.h"

TEST_CASE("the BTOF charge sharing algorithm runs", "[TOFPulseDigitization]") {
  const float EPSILON = 1e-5;

  eicrecon::TOFPulseDigitization algo("TOFPulseDigitization");

  std::shared_ptr<spdlog::logger> logger = spdlog::default_logger()->clone("TOFPulseDigitization");
  logger->set_level(spdlog::level::trace);

  eicrecon::TOFHitDigiConfig cfg;
  cfg.readout = "MockTOFHits";

  auto detector = algorithms::GeoSvc::instance().detector();
  auto id_desc  = detector->readout(cfg.readout).idSpec();

  cfg.gain         = 10;
  cfg.Vm           = -1e-4;
  cfg.ignore_thres = 1e-4 / 5;
  cfg.t_thres      = cfg.Vm * 0.1;
  cfg.tdc_bit      = 8;
  cfg.adc_bit      = 7;
  cfg.tdc_range = pow(2, cfg.tdc_bit);
  cfg.adc_range             = pow(2, cfg.adc_bit);

  // check if max pulse height is linearly proportional to the initial Edep
  algo.applyConfig(cfg);
  algo.init();

  SECTION("TDC vs analytic solution scan") {
    logger->info("Begin TDC vs analytic solution scan");

    // test pulse with gaussian shape
    for (double tdc_frac = 0.4; tdc_frac < 1; tdc_frac += 0.1) {
      edm4hep::RawTimeSeriesCollection time_series_coll;
      auto rawhits_coll = std::make_unique<edm4eic::RawTrackerHitCollection>();

      auto pulse = time_series_coll.create();
      auto cellID =
          id_desc.encode({{"system", 0}, {"module", 0}, {"sensor", 1}, {"x", 1}, {"y", 1}});

      pulse.setCellID(cellID);
      pulse.setCharge(1.); // placeholder
      pulse.setTime(1.);   // placeholder
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
      REQUIRE(hit.getTimeStamp() == analytic_TDC);
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
      pulse.setTime(1.);   // placeholder
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
