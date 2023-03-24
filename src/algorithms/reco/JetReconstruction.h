// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Derek Anderson, Zhongling Ji

#ifndef EICRECON_JETRECONSTRUCTION_H
#define EICRECON_JETRECONSTRUCTION_H

// standard c includes
#include <vector>
// DD4hep includes
#include <spdlog/spdlog.h>
#include <DD4hep/DD4hepUnits.h>
#include <edm4hep/utils/kinematics.h>
#include <edm4eic/ReconstructedParticle.h>
// fastjet includes
#include <fastjet/config.h>
#include <fastjet/JetDefinition.hh>
#include <fastjet/AreaDefinition.hh>



namespace eicrecon {

  // jet reco configuration
  class JetReconstruction {

    public:

      void init(std::shared_ptr<spdlog::logger> logger);

      std::vector<edm4eic::ReconstructedParticle*> execute(
          std::vector<const edm4hep::LorentzVectorE*> momenta
          );

      // input parameters
      // FIXME these will need to adjusted...
      double m_minCstPt  = 0.5 * dd4hep::MeV;  // minimum pT of objects fed to cluster sequence [set to just under e- mass right now]
      double m_maxCstPt  = 50. * dd4hep::GeV;  // maximum pT of objects fed to clsuter sequence

      // jet parameters
      float                         m_rJet         = 0.4;                             // jet resolution  parameter
      double                        m_minJetPt     = m_minCstPt;                      // minimum jet pT
      fastjet::JetAlgorithm         m_jetAlgo      = fastjet::antikt_algorithm;       // jet finding algorithm
      fastjet::RecombinationScheme  m_recombScheme = fastjet::RecombinationScheme::pt_scheme;  // particle recombination scheme

      // area parameters
      double             m_ghostMaxRap    = 5.;                     // maximum rapidity of ghosts
      int                m_numGhostRepeat = 1;                      // number of times a ghost is reused per grid site
      double             m_ghostArea      = 0.001;                  // area per ghost
      fastjet::AreaType  m_areaType       = fastjet::AreaType::active_area;  // type of area calculated

    private:

      std::shared_ptr<spdlog::logger> m_log;

  };  // end config struct

}  // end eicrecon namespace

#endif
