// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Chun Yuen Tsang, Prithwish Tribedy

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <Evaluator/DD4hepUnits.h>
#include <algorithms/geo.h>
#include <catch2/catch_test_macros.hpp> // for AssertionHandler, operator""_catch_sr, StringRef, REQUIRE, operator<, operator==, operator>, TEST_CASE
#include <edm4hep/RawTimeSeriesCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <fmt/core.h>
#include <podio/RelationRange.h>
#include <spdlog/common.h> // for level_enum
#include <spdlog/logger.h> // for logger
#include <spdlog/spdlog.h> // for default_logger
#include <algorithm>
#include <cmath>
#include <gsl/pointers>
#include <limits>
#include <memory> // for allocator, unique_ptr, make_unique, shared_ptr, __shared_ptr_access
#include <tuple>
#include <utility>
#include <vector>

#include "TF1.h"
#include "TGraphErrors.h"
#include "algorithms/digi/LGADPulseGeneration.h"
#include "algorithms/digi/LGADPulseGenerationConfig.h"

TEST_CASE("the LGAD charge sharing algorithm runs", "[LGADPulseGeneration]") {
  const float EPSILON = 1e-5;
  eicrecon::LGADPulseGenerationConfig cfg;
  cfg.gain         = 113;
  cfg.Vm           = 1e-4 * dd4hep::GeV;
  cfg.ignore_thres = 1e-4 / 5;
  cfg.sigma_analog = 0.293951 * dd4hep::ns;
  cfg.adc_bit      = 8;
  cfg.adc_range    = pow(2, cfg.adc_bit);

  eicrecon::LGADPulseGeneration algo("LGADPulseGeneration");

  std::shared_ptr<spdlog::logger> logger = spdlog::default_logger()->clone("LGADPulseGeneration");
  logger->set_level(spdlog::level::trace);


  auto detector = algorithms::GeoSvc::instance().detector();
  auto id_desc  = detector->readout("MockLGADHits").idSpec();

  SECTION("Pulse height linearlity test") {
    // check if max pulse height is linearly proportional to the initial Edep
    algo.applyConfig(cfg);
    algo.init();

    TGraphErrors graph;
    for (double edep = 0; edep <= 1e-4; edep += 1e-4 / 9) {
      edm4hep::SimTrackerHitCollection rawhits_coll;
      auto time_series_coll = std::make_unique<edm4hep::RawTimeSeriesCollection>();

      auto hit = rawhits_coll.create();
      auto cellID =
          id_desc.encode({{"system", 0}, {"module", 0}, {"sensor", 1}, {"x", 1}, {"y", 1}});

      hit.setCellID(cellID);
      hit.setEDep(
          edep); // in GeV. Since Vm = 1e-4*gain, EDep = 1e-4 GeV corresponds to ADC = max_adc
      hit.setTime(1.5 * dd4hep::ns); // in ns

      // Constructing input and output as per the algorithm's expected signature
      auto input  = std::make_tuple(&rawhits_coll);
      auto output = std::make_tuple(time_series_coll.get());

      algo.process(input, output);

      if (edep < 1e-4 / 5)
        REQUIRE(time_series_coll->size() == 0);
      else {
        REQUIRE(time_series_coll->size() == 1);
        auto min_adc = std::numeric_limits<int>::max();
        for(const auto& pulse : (*time_series_coll)) {
          REQUIRE(pulse.getCellID() == cellID);
          auto adcs    = pulse.getAdcCounts();
          for (const auto adc : adcs)
            min_adc = std::min(min_adc, adc);
        }
        int npt = graph.GetN();
        graph.SetPoint(npt, edep, min_adc);
        graph.SetPointError(npt, 0, 0.5);
        // make sure when energy deposition = Vm, ADC reaches max value
        if (edep == 1e-4)
          REQUIRE(min_adc == -cfg.adc_range + 1);
      }
    }

    // test linearlity
    TF1 tf1("tf1", "pol1", 0, 1e-4);
    graph.Fit(&tf1, "R0");
    // slope can't be consistent with zero
    REQUIRE(!(tf1.GetParameter(1) - tf1.GetParError(1) < 0 && 0 < tf1.GetParameter(1) + tf1.GetParError(1) ));
    double chi2_dof = tf1.GetChisquare() / tf1.GetNDF();
    logger->info("Chi-square/dof value for Edep vs min-adc = {}", chi2_dof);
    REQUIRE(chi2_dof < 2);
  }

  SECTION("Pulse timing linearlity test") {
    // check if max pulse height is linearly proportional to the initial Edep
    algo.applyConfig(cfg);
    algo.init();

    TGraphErrors graph;
    std::vector<double> times;

    // test within the same EICROC cycle
    for (double time = 0; time < 12; time += 1.) times.push_back(time);
    // test multiple EICROC cycle
    for (double time = 10; time < 101; time += 25.) times.push_back(time);
    // test negative time
    times.push_back(-10);

    for (double time : times) {
      edm4hep::SimTrackerHitCollection rawhits_coll;
      auto time_series_coll = std::make_unique<edm4hep::RawTimeSeriesCollection>();

      auto hit = rawhits_coll.create();
      auto cellID =
          id_desc.encode({{"system", 0}, {"module", 0}, {"sensor", 1}, {"x", 1}, {"y", 1}});

      hit.setCellID(cellID);
      hit.setEDep(
          0.5e-4 * dd4hep::GeV); // in GeV. Since Vm = 1e-4*gain, EDep = 1e-4 GeV corresponds to ADC = max_adc
      hit.setTime(time); // in ns

      // Constructing input and output as per the algorithm's expected signature
      auto input  = std::make_tuple(&rawhits_coll);
      auto output = std::make_tuple(time_series_coll.get());

      algo.process(input, output);

      REQUIRE(time_series_coll->size() == 1);
      auto min_adc          = std::numeric_limits<int>::max();
      int time_bin = 0;
      for(const auto& pulse: *time_series_coll) {
        //auto pulse = (*time_series_coll)[0];
        REQUIRE(pulse.getCellID() == cellID);

        auto adcs             = pulse.getAdcCounts();
        for (unsigned int i = 0; i < adcs.size(); ++i) {
          auto adc = adcs[i];
          if (adc < min_adc)
            time_bin = i + pulse.getTime()/cfg.tMax*cfg.tdc_range;
          min_adc = std::min(min_adc, adc);
        }
      }
      int npt = graph.GetN();
      graph.SetPoint(npt, time, time_bin);
      graph.SetPointError(npt, 0, 0.5);
    }

    // test linearlity
    TF1 tf1("tf1", "pol1", *std::min_element(times.begin(), times.end()), *std::max_element(times.begin(), times.end()));
    graph.Fit(&tf1, "R0");
    // slope can't be consistent with zero
    REQUIRE(!(tf1.GetParameter(1) - tf1.GetParError(1) < 0 && 0 < tf1.GetParameter(1) + tf1.GetParError(1) ));
    double chi2_dof = tf1.GetChisquare() / tf1.GetNDF();
    logger->info("Chi-square/dof value for time vs TDC-bin = {}", chi2_dof);
    REQUIRE(chi2_dof < 2);
  }
}
