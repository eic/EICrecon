// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#pragma once

#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzE4D.h>
#include <Math/Vector4Dfwd.h>
#include <cmath>

namespace eicrecon {

/// Summed quantities of the hadronic final state
struct HadronicFinalStateSums {
  double sigma;
  double pT;
  double gamma;
};

/// Summed quantities of the hadronic final state, from the sum of its four-momenta in the
/// head-on frame. An empty hadronic final state (sigma = pT = 0) has gamma = 0, the limit for
/// hadrons along the hadron beam.
inline HadronicFinalStateSums hadronic_final_state_sums(const ROOT::Math::PxPyPzEVector& sum) {
  const auto sigma = sum.E() - sum.Pz();
  const auto pT    = sum.Pt();
  const auto denom = pT * pT + sigma * sigma;
  const auto gamma = (denom > 0 ? std::acos((pT * pT - sigma * sigma) / denom) : 0.);
  return {sigma, pT, gamma};
}

} // namespace eicrecon
