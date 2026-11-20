// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Dmitry Romanov

#pragma once

#include <edm4eic/unit_system.h>
#include <numbers>

namespace eicrecon {

struct ChargedLambdaReconstructionConfig {

  /** maximum opening angle between the two daughter momenta [rad] */
  double openingAngleMax = 34e-3;
  /** maximum polar angle of the summed pair momentum [rad]; Lambdas from ep events
   *  that decay in the far-forward region are within tens of mrad of the beam */
  double pairThetaMax = 57e-3;
  /** maximum distance of closest approach between the two daughter track lines [mm];
   *  straight-line approximation, see ChargedLambdaReconstruction.h */
  double dcaMax = 90.0 * edm4eic::unit::mm;
  /** accepted z range of the two-line closest-approach midpoint (decay-vertex proxy) [mm].
   *  The proxy follows the true decay z out to the B0 reach (~6 m) for B0 daughters; the
   *  window is wider to keep pairings with Roman-Pot and off-momentum protons, for which
   *  the proxy is weakly constrained (see ChargedLambdaReconstruction.h) */
  double vertexZMin = -1.5 * edm4eic::unit::m;
  double vertexZMax = 32.0 * edm4eic::unit::m;
  /** maximum angle between the pair momentum and the direction from the origin to the
   *  decay-vertex proxy [rad]. Disabled by default (pi): Roman-Pot and off-momentum
   *  candidates combine a transfer-matrix momentum at the IP with a reference point at
   *  their station, so their straight line does not locate the decay. The variable
   *  becomes meaningful once far-forward candidates carry an independent vertex estimate. */
  double pointingMax = std::numbers::pi;
  /** minimum momentum asymmetry (|p_p| - |p_pi|) / (|p_p| + |p_pi|); the proton carries
   *  approximately m_p/m_Lambda of the Lambda momentum, so signal sits at 0.7-0.9 */
  double momAsymMin = 0.47;
  /** half-width of the accepted invariant-mass window around the Lambda mass [GeV] */
  double massWindow = 25.0 * edm4eic::unit::MeV;
};

} // namespace eicrecon
