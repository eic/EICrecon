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
  /** When true the seed perigee is placed at the MCParticle vertex instead
   *  of being extrapolated back to the beam axis.  This is essential for
   *  particles produced at large |z| (e.g. Lambda decay daughters) where
   *  the standard back-extrapolation puts the reference point far behind
   *  the IP, forcing ACTS to propagate many metres before reaching the
   *  detector, which degrades or eliminates CKF efficiency.
   */
  bool useVertexAsPerigee = false;
};
