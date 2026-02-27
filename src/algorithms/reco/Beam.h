// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck

#pragma once

#include <Math/Vector4D.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <algorithm>
#include <set>
#include <vector>
#include <cmath>

using ROOT::Math::PxPyPzEVector;

namespace eicrecon {

template <class T> auto find_first_with_pdg(const T* parts, const std::set<int32_t>& pdg) {
  T c;
  c.setSubsetCollection();
  const auto it = std::find_if(parts->begin(), parts->end(),
                               [&pdg](const auto& p) { return pdg.count(p.getPDG()) > 0; });
  if (it != parts->end()) {
    c.push_back(*it);
  }
  return c;
}

template <class T>
auto find_first_with_status_pdg(const T* parts, const std::set<int32_t>& status,
                                const std::set<int32_t>& pdg) {
  T c;
  c.setSubsetCollection();
  const auto it = std::find_if(parts->begin(), parts->end(), [&status, &pdg](const auto& p) {
    return status.count(p.getGeneratorStatus()) > 0 && pdg.count(p.getPDG()) > 0;
  });
  if (it != parts->end()) {
    c.push_back(*it);
  }
  return c;
}

inline auto find_first_beam_electron(const edm4hep::MCParticleCollection* mcparts) {
  return find_first_with_status_pdg(mcparts, {4}, {11});
}

inline auto find_first_beam_hadron(const edm4hep::MCParticleCollection* mcparts) {
  return find_first_with_status_pdg(mcparts, {4}, {2212, 2112});
}

inline auto find_first_scattered_electron(const edm4hep::MCParticleCollection* mcparts) {
  return find_first_with_status_pdg(mcparts, {1}, {11});
}

inline auto find_first_scattered_electron(const edm4eic::ReconstructedParticleCollection* rcparts) {
  return find_first_with_pdg(rcparts, {11});
}

template <typename Vector3>
PxPyPzEVector round_beam_four_momentum(const Vector3& p_in, const float mass,
                                       const std::vector<float>& pz_set,
                                       const float crossing_angle = 0.0) {
  PxPyPzEVector p_out;
  for (const auto& pz : pz_set) {
    if (std::abs(p_in.z / pz - 1) < 0.1) {
      p_out.SetPz(pz);
      break;
    }
  }
  p_out.SetPx(p_out.Pz() * sin(crossing_angle));
  p_out.SetPz(p_out.Pz() * cos(crossing_angle));
  p_out.SetE(std::hypot(p_out.Px(), p_out.Pz(), mass));
  return p_out;
}

} // namespace eicrecon
