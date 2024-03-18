// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner

#pragma once

namespace eicrecon {

  struct MatrixTransferStaticConfig {

    float     partMass  {0.938272};
    float     partCharge{1};
    long long partPDG   {2212};

    // Defaults here are for RPOTS
    double local_x_offset      {0.0};
    double local_y_offset      {0.0};
    double local_x_slope_offset{-0.00622147};
    double local_y_slope_offset{-0.0451035};
    double crossingAngle       {0.025};
    double nomMomentum         {100.0};

    //std::vector<std::vector<double>> aX = {{2.102403743, 29.11067626},
    //                                       {0.186640381, 0.192604619}};
    //std::vector<std::vector<double>> aY = {{0.0000159900, 3.94082098},
    //                                       {0.0000079946, -0.1402995}};


    //x_offset       = 0.00979216;
    //y_offset       = -0.00778646;
    //x_slope_offset = 0.004526961;
    //y_slope_offset = -0.003907849;

    std::vector<std::vector<double>> aX = {{2.03459216, 22.85780784},
                                           {0.179641961, -0.306626961}};
    std::vector<std::vector<double>> aY = {{0.38879, 3.71612646},
                                           {0.022685, -0.003907849}};

    double hit1minZ{0};
    double hit1maxZ{0};
    double hit2minZ{0};
    double hit2maxZ{0};

    std::string readout{""};

  };

}
