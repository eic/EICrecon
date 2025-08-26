// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <Evaluator/DD4hepUnits.h>

struct TrackParamTruthInitConfig {

  double maxVertexX     = 80 * dd4hep::mm;
  double maxVertexY     = 80 * dd4hep::mm;
  double maxVertexZ     = 200 * dd4hep::mm;
  double minMomentum    = 100 * dd4hep::MeV;
  double maxEtaForward  = 6.0;
  double maxEtaBackward = 4.1;
  double momentumSmear  = 0.1;
};
