// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Daniel Brandenburg, Dmitry Kalinkin, Stephen Maple

#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzM4D.h>
#include <Math/Vector4Dfwd.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <functional>
#include <gsl/pointers>
#include <map>
#include <utility>

#include "algorithms/reco/ScatteredElectronsEMinusPz.h"
#include "algorithms/reco/ScatteredElectronsEMinusPzConfig.h"

#include "algorithms/reco/ElectronFinderUtils.h"

using ROOT::Math::PxPyPzEVector;
using ROOT::Math::PxPyPzMVector;

namespace eicrecon {

/**
   * @brief Initialize the ScatteredElectronsEMinusPz Algorithm
   */
void ScatteredElectronsEMinusPz::init() {}

/**
   * @brief Produce a list of scattered electron candidates
   *
   * @param rcparts - input collection of all reconstructed particles
   * @param rcele  - input collection of all electron candidates
   * @return std::unique_ptr<edm4eic::ReconstructedParticleCollection>
   */
void ScatteredElectronsEMinusPz::process(const ScatteredElectronsEMinusPz::Input& input,
                                         const ScatteredElectronsEMinusPz::Output& output) const {
  auto [rcparts, rcele] = input;
  auto [out_electrons]  = output;

  static const auto m_electron = m_particleSvc.particle(11).mass;
  static const auto m_pion     = m_particleSvc.particle(211).mass;

  // this map will store intermediate results
  // so that we can sort them before filling output
  // collection
  std::map<double, edm4eic::ReconstructedParticle, std::greater<>> scatteredElectronsMap;

  out_electrons->setSubsetCollection();

  trace("We have {} candidate electrons", rcele->size());

  // Lorentz Vector for the scattered electron,
  // hadronic final state, and individual hadron
  // We do it here to avoid creating objects inside the loops
  PxPyPzMVector vScatteredElectron;
  PxPyPzMVector vHadronicFinalState;
  PxPyPzMVector vHadron;

  for (const auto& e : *rcele) {
    // Do not cut on charge to account for charge-symmetric background

    // Cut on isolation
    double isolation = calc_isolation(e, *rcparts, m_cfg.isolationR);
    if (isolation < m_cfg.minIsolation)
      continue;

    // reset the HadronicFinalState
    vHadronicFinalState.SetCoordinates(0, 0, 0, 0);

    // Set a vector for the electron we are considering now
    vScatteredElectron.SetCoordinates(e.getMomentum().x, e.getMomentum().y, e.getMomentum().z,
                                      m_electron);

    // Loop over reconstructed particles to
    // sum hadronic final state
    for (const auto& p : *rcparts) {
      // What we want is to add all reconstructed particles
      // except the one we are currently considering as the
      // (scattered) electron candidate.
      if (p.getObjectID() != e.getObjectID()) {
        vHadron.SetCoordinates(p.getMomentum().x, p.getMomentum().y, p.getMomentum().z,
                               m_pion // Assume pion for hadronic state
        );

        // Sum hadronic final state
        vHadronicFinalState += vHadron;
      } else {
        trace("Skipping electron in hadronic final state");
      }
    } // hadron loop (reconstructed particles)

    // Calculate the E-Pz for this electron
    // + hadron final state combination
    // For now we keep all electron
    // candidates but we will rank them by their E-Pz
    double EPz = (vScatteredElectron + vHadronicFinalState).E() -
                 (vScatteredElectron + vHadronicFinalState).Pz();
    trace("\tE-Pz={}", EPz);
    trace("\tScatteredElectron has Pxyz=( {}, {}, {} )", e.getMomentum().x, e.getMomentum().y,
          e.getMomentum().z);

    // Store the result of this calculation
    scatteredElectronsMap[EPz] = e;
  } // electron loop

  trace("Selecting candidates with {} < E-Pz < {}", m_cfg.minEMinusPz, m_cfg.maxEMinusPz);

  bool first = true;
  // map defined with std::greater<> will be iterated in descending order by the key
  for (auto kv : scatteredElectronsMap) {

    double EMinusPz = kv.first;
    // Do not save electron candidates that
    // are not within range
    if (EMinusPz > m_cfg.maxEMinusPz || EMinusPz < m_cfg.minEMinusPz) {
      continue;
    }

    // For logging and development
    // report the highest E-Pz candidate chosen
    if (first) {
      trace("Max E-Pz Candidate:");
      trace("\tE-Pz={}", EMinusPz);
      trace("\tScatteredElectron has Pxyz=( {}, {}, {} )", kv.second.getMomentum().x,
            kv.second.getMomentum().y, kv.second.getMomentum().z);
      first = false;
    }
    out_electrons->push_back(kv.second);
  } // reverse loop on scatteredElectronsMap
}

} // namespace eicrecon
