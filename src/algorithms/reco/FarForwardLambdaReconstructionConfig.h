// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Sebouh Paul
#pragma once
#include <float.h>
#include <DD4hep/Detector.h>


namespace eicrecon {

  struct FarForwardLambdaReconstructionConfig {
    /** transformation from global coordinates to proton-frame coordinates*/
    double globalToProtonRotation=-0.025;
    /** distance to the ZDC */
    double zMax=35800*dd4hep::mm;
    /** maximum deviation between reconstructed mass and PDG mass */
    double lambdaMaxMassDev=0.030*dd4hep::GeV;
    /** number of iterations for the IDOLA algorithm */
    int iterations=10;
  };

} // eicrecon
