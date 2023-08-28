// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023, Christopher Dilks, Luigi Dello Stritto

#pragma once

#include <spdlog/spdlog.h>

namespace eicrecon {
  struct MatrixTransferStaticConfig {
    
    float partMass  {0.938272};
    float partCharge{1};
    float partPDG   {2122};

    double local_x_offset      {0.0};
    double local_y_offset      {0.0};
    double local_x_slope_offset{-0.00622147};
    double local_y_slope_offset{-0.0451035};
    double crossingAngle       {0.025};
    double nomMomentum         {275.0};
      
    std::vector<std::vector<double>> aX = {{2.102403743, 29.11067626},
					   {0.186640381, 0.192604619}};
    std::vector<std::vector<double>> aY = {{0.0000159900, 3.94082098},
					   {0.0000079946, -0.1402995}};
    
/*       double aX[2][2] = {{2.102403743, 29.11067626}, */
/* 			 {0.186640381, 0.192604619}}; */
/*       double aY[2][2] = {{0.0000159900, 3.94082098}, */
/* 			 {0.0000079946, -0.1402995}}; */
      
/*       // ------------------------------------------------ */

/*       // print all parameters */
/*       void Print(std::shared_ptr<spdlog::logger> m_log, spdlog::level::level_enum lvl=spdlog::level::debug) { */
/*         m_log->log(lvl, "{:=^60}"," PhotoMultiplierHitDigiConfig Settings "); */
/*         auto print_param = [&m_log, &lvl] (auto name, auto val) { */
/*           m_log->log(lvl, "  {:>20} = {:<}", name, val); */
/*         }; */
/*         print_param("seed",seed); */
/*         print_param("hitTimeWindow",hitTimeWindow); */
/*         print_param("timeResolution",timeResolution); */
/*         print_param("speMean",speMean); */
/*         print_param("speError",speError); */
/*         print_param("pedMean",pedMean); */
/*         print_param("pedError",pedError); */
/*         print_param("enablePixelGaps",enablePixelGaps); */
/*         print_param("pixelSize",pixelSize); */
/*         print_param("safetyFactor",safetyFactor); */
/*         print_param("enableNoise",enableNoise); */
/*         print_param("noiseRate",noiseRate); */
/*         print_param("noiseTimeWindow",noiseTimeWindow); */
/*         m_log->log(lvl, "{:-^60}"," Quantum Efficiency vs. Wavelength "); */
/*         for(auto& [wl,qe] : quantumEfficiency) */
/*           m_log->log(lvl, "  {:>10} {:<}",wl,qe); */
/*       } */

  };
}
