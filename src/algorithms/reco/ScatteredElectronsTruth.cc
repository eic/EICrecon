// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Daniel Brandenburg

#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzE4D.h>
#include <Math/GenVector/PxPyPzM4D.h>
#include <Math/Vector4Dfwd.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <podio/ObjectID.h>
#include <gsl/pointers>
#include <vector>

#include "algorithms/reco/Beam.h"
#include "algorithms/reco/ScatteredElectronsTruth.h"

using ROOT::Math::PxPyPzEVector;
using ROOT::Math::PxPyPzMVector;

namespace eicrecon {

/**
   * @brief Initialize ScatteredElectronsTruth algorithm
   */
void ScatteredElectronsTruth::init() {}

/**
   * @brief Selects the scattered electron based
   * on TRUTH information (Mc particle info).
   * Returns a collection of reconstructed particles
   * corresponding to the chosen Mc electrons
   *
   * @param input - McParticleCollection,
   *                ReconstructedParticleCollection,
   *                MCRecoParticleAssociationCollection
   * @param output - ReconstructedParticleCollection
   */
void ScatteredElectronsTruth::process(const ScatteredElectronsTruth::Input& input,
                                      const ScatteredElectronsTruth::Output& output) const {

  // get our input and outputs
  const auto [mcparts, rcparts, rcassoc] = input;
  auto [output_electrons]                = output;
  output_electrons->setSubsetCollection();

  // Get first scattered electron
  const auto ef_coll = find_first_scattered_electron(mcparts);
  if (ef_coll.empty()) {
    trace("No truth scattered electron found");
    return;
  }

  // Associate first scattered electron
  // with reconstructed electron
  auto ef_assoc = rcassoc->begin();
  for (; ef_assoc != rcassoc->end(); ++ef_assoc) {
    if (ef_assoc->getSim().getObjectID() == ef_coll[0].getObjectID()) {
      break;
    }
  }

  // Check to see if the associated reconstructed
  // particle is available
  if (!(ef_assoc != rcassoc->end())) {
    trace("Truth scattered electron not in reconstructed particles");
    return;
  }

  // Get the reconstructed electron object
  const auto ef_rc{ef_assoc->getRec()};
  const auto ef_rc_id{ef_rc.getObjectID()};

  // Use these to compute the E-Pz
  // This is for development of the EMinusPz
  // algorithm
  PxPyPzMVector vScatteredElectron;
  PxPyPzMVector vHadronicFinalState;
  PxPyPzMVector vHadron;

  // Loop over reconstructed particles to
  // get outgoing scattered electron.
  // Use the true scattered electron from the
  // MC information
  std::vector<PxPyPzEVector> electrons;
  for (const auto& p : *rcparts) {
    if (p.getObjectID() == ef_rc_id) {

      output_electrons->push_back(p);
      static const auto m_electron = m_particleSvc.particle(11).mass;
      vScatteredElectron.SetCoordinates(p.getMomentum().x, p.getMomentum().y, p.getMomentum().z,
                                        m_electron);
      electrons.emplace_back(p.getMomentum().x, p.getMomentum().y, p.getMomentum().z,
                             p.getEnergy());
      // break; NOTE: if we are not computing E-Pz
      // we can safely break here and save precious CPUs
    } else {
      // Compute the sum hadronic final state
      static const auto m_pion = m_particleSvc.particle(211).mass;
      vHadron.SetCoordinates(p.getMomentum().x, p.getMomentum().y, p.getMomentum().z, m_pion);
      vHadronicFinalState += vHadron;
    }
  }

  // If no scattered electron was found, too bad
  if (electrons.empty()) {
    trace("No Truth scattered electron found");
    return;
  }

  // Just for debug/development
  // report the computed E-Pz for the chosen electron
  double EPz = (vScatteredElectron + vHadronicFinalState).E() -
               (vScatteredElectron + vHadronicFinalState).Pz();
  trace("We found {} scattered electrons using Truth association", electrons.size());
  trace("TRUTH scattered electron has E-Pz = {}", EPz);
  trace("TRUTH scattered electron has Pxyz=( {}, {}, {} ) and E/p = {}", electrons[0].Px(),
        electrons[0].Py(), electrons[0].Pz(), (electrons[0].E() / electrons[0].P()));
} // process

} // namespace eicrecon
