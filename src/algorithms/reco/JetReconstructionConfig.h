// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory
//
// ----------------------------------------------------------------------------
// 'JetReconstructionConfig.h'
// Derek Anderson
// 02.10.2023
//
// configures parameters for jet reconstruction algorithm
// ---------------------------------------------------------------------------

#ifndef EICRECON_JETRECONSTRUCTION_CONFIG_H
#define EICRECON_JETRECONSTRUCTION_CONFIG_H

// standard c includes
#include <vector>
// fastjet includes
#include "fastjet/config.h"
#include "fastjet/Selector.hh"
#include "fastjet/PseudoJet.hh"
#include "fastjet/ClusterSequence.hh"
#include "fastjet/ClusterSequenceArea.hh"
#include "fastjet/tools/Subtractor.hh"
#include "fastjet/tools/JetMedianBackgroundEstimator.hh"
// jet definition
#include <edm4eic/Jet.h>

using namespace fastjet;



namespace eicrecon {

  // jet reco configuration
  struct JetReconstructionConfig {

    // input parameters
    // FIXME these will need to adjusted...
    double m_minCstPt  = 0.5 * dd4hep::MeV;  // minimum pT of objects fed to cluster sequence [set to just under e- mass right now]
    double m_maxCstPt  = 50. * dd4hep::GeV;  // maximum pT of objects fed to clsuter sequence

    // jet parameters
    float                m_rJet         = 0.4;                             // jet resolution  parameter
    double               m_minJetPt     = m_minCstPt;                      // minimum jet pT
    Algorithm            m_jetAlgo      = Algorithm::antikt_algorithm;     // jet finding algorithm
    RecombinationScheme  m_recombScheme = RecombinationScheme::pt_scheme;  // particle recombination scheme

    // area parameters
    double    m_ghostMaxRap    = 5.;                     // maximum rapidity of ghosts
    int       m_numGhostRepeat = 1;                      // number of times a ghost is reused per grid site
    double    m_ghostArea      = 0.001;                  // area per ghost
    AreaType  m_areaType       = AreaType::active_area;  // type of area calculated

  };  // end config struct

}  // end eicrecon namespace

#endif

// end ------------------------------------------------------------------------
