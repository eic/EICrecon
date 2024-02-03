// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Daniel Brandenburg

#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzE4D.h>
#include <Math/Vector4Dfwd.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <cmath>
#include <gsl/pointers>
#include <vector>
#include <TLorentzVector.h>

#include "Beam.h"
#include "ScatteredElectronsEMinusPz.h"

using ROOT::Math::PxPyPzEVector;
using ROOT::Math::PxPyPzMVector;

namespace eicrecon {

  /**
   * @brief Initialize the ScatteredElectronsEMinusPz Algorithm
   *
   * @param logger
   */
  void ScatteredElectronsEMinusPz::init(std::shared_ptr<spdlog::logger>& logger) {
    m_log = logger;
  }

  /**
   * @brief Produce a list of scattered electron candidates
   *
   * @param rcparts - input collection of all reconstructed particles
   * @param rcele  - input collection of all electron candidates
   * @return std::unique_ptr<edm4eic::ReconstructedParticleCollection>
   */
  std::unique_ptr<edm4eic::ReconstructedParticleCollection> ScatteredElectronsEMinusPz::execute(
                const edm4eic::ReconstructedParticleCollection *rcparts,
                const edm4eic::ReconstructedParticleCollection *rcele
        ){

    // this map will store intermediate results
    // so that we can sort them before filling output
    // collection
    std::map<double, edm4eic::MutableReconstructedParticle> scatteredElectronsMap;

    // our output collection of scattered electrons
    // ordered by E-Pz
    auto out_electrons =  std::make_unique<
        edm4eic::ReconstructedParticleCollection
      >();

    m_log->trace( "We have {} candidate electrons",
        rcele->size()
      );

    // Lorentz Vector for the scattered electron,
    // hadronic final state, and individual hadron
    // We do it here to avoid creating objects inside the loops
    PxPyPzMVector  vScatteredElectron;
    PxPyPzMVector  vHadronicFinalState;
    PxPyPzMVector  vHadron;

    for ( const auto& e: *rcele ) {
      // Only consider electrons (ReconstructedElectrons includes positrons as well)
      if (e.getCharge() != -1) {
        m_log->trace( "skipping positron" );
        continue;
      }

      // reset the HadronicFinalState
      vHadronicFinalState.SetCoordinates(0, 0, 0, 0);

      // Set a vector for the electron we are considering now
      vScatteredElectron.SetCoordinates(
          e.getMomentum().x,
          e.getMomentum().y,
          e.getMomentum().z,
          m_electron
        );

      // Loop over reconstructed particles to
      // sum hadronic final state
      for (const auto& p: *rcparts) {
        // this is a hack - getObjectID() only works within
        // a collections, not unique across all collections.
        // What we want is to add all reconstructed particles
        // except the one we are currently considering as the
        // (scattered) electron candidate.
        // This does work though and in general it has only
        // one match as I would hope (tested on pythia events)
        if ( edm4hep::utils::magnitude(p.getMomentum())
          != edm4hep::utils::magnitude(e.getMomentum()) ) {
          vHadron.SetCoordinates(
              p.getMomentum().x,
              p.getMomentum().y,
              p.getMomentum().z,
              m_pion // Assume pion for hadronic state
            );

          // Sum hadronic final state
          vHadronicFinalState += vHadron;
        } else {
          m_log->trace( "Skipping electron in hadronic final state" );
        }
      } // hadron loop (reconstructed particles)

      // Calculate the E-Pz for this electron
      // + hadron final state combination
      // For now we keep all electron
      // candidates but we will rank them by their E-Pz
      double EPz=(vScatteredElectron+vHadronicFinalState).E()
              - (vScatteredElectron+vHadronicFinalState).Pz();
      m_log->trace( "\tE-Pz={}", EPz );
      m_log->trace( "\tScatteredElectron has Pxyz=( {}, {}, {} )", e.getMomentum().x, e.getMomentum().y, e.getMomentum().z );

      // Store the result of this calculation
      scatteredElectronsMap[ EPz ] = e.clone();
    } // electron loop

    m_log->trace( "Selecting candidates with {} < E-Pz < {}", m_cfg.minEMinusPz, m_cfg.maxEMinusPz );

    // map sorts in descending order by default
    // sort by descending
    bool first = true;
    // for (auto kv : scatteredElectronsMap) {
    for (auto kv = scatteredElectronsMap.rbegin(); kv != scatteredElectronsMap.rend(); ++kv) {

      double EMinusPz = kv->first;
      // Do not save electron candidates that
      // are not within range
      if ( EMinusPz > m_cfg.maxEMinusPz
        || EMinusPz < m_cfg.minEMinusPz ){
        continue;
      }

      // For logging and development
      // report the highest E-Pz candidate chosen
      if ( first ){
        m_log->trace( "Max E-Pz Candidate:" );
        m_log->trace( "\tE-Pz={}", EMinusPz );
        m_log->trace( "\tScatteredElectron has Pxyz=( {}, {}, {} )", kv->second.getMomentum().x, kv->second.getMomentum().y, kv->second.getMomentum().z );
        first = false;
      }
      out_electrons->push_back( kv->second );
    } // reverse loop on scatteredElectronsMap


    // Return Electron candidates ranked
    // in order from largest E-Pz to smallest
    return out_electrons;
  }

} // namespace eicrecon
