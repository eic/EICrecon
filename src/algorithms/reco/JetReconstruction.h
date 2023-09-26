// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Derek Anderson, Zhongling Ji

#ifndef EICRECON_JETRECONSTRUCTION_H
#define EICRECON_JETRECONSTRUCTION_H

#include <vector>
#include <spdlog/spdlog.h>
#include <DD4hep/DD4hepUnits.h>
// event data model definitions
#include <edm4hep/utils/kinematics.h>
#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/ReconstructedParticleCollection.h>
// for fast jet objects
#include <fastjet/config.h>
#include <fastjet/JetDefinition.hh>
#include <fastjet/AreaDefinition.hh>
// for algorithm configuration
#include "algorithms/interfaces/WithPodConfig.h"
#include "JetReconstructionConfig.h"

namespace eicrecon {

  class JetReconstruction : public WithPodConfig<JetReconstructionConfig> {

    public:

      void init(std::shared_ptr<spdlog::logger> logger);
      std::unique_ptr<edm4eic::ReconstructedParticleCollection> process(
        const std::vector<const edm4hep::LorentzVectorE*> momenta
      );

    private:

      // input parameters
      double m_minCstPt  = 0.2  * dd4hep::GeV;  // minimum pT of objects fed to cluster sequence
      double m_maxCstPt  = 100. * dd4hep::GeV;  // maximum pT of objects fed to clsuter sequence

      // jet parameters
      float  m_rJet     = 1.0;                // jet resolution  parameter
      double m_minJetPt = 1.0 * dd4hep::GeV;  // minimum jet pT

      // area parameters
      int    m_numGhostRepeat = 1;      // number of times a ghost is reused per grid site
      double m_ghostMaxRap    = 3.5;    // maximum rapidity of ghosts
      double m_ghostArea      = 0.001;  // area per ghost

      // fastjet options
      fastjet::JetAlgorithm         m_jetAlgo      = fastjet::antikt_algorithm;               // jet finding algorithm
      fastjet::RecombinationScheme  m_recombScheme = fastjet::RecombinationScheme::E_scheme;  // particle recombination scheme
      fastjet::AreaType             m_areaType     = fastjet::AreaType::active_area;          // type of area calculated

      std::shared_ptr<spdlog::logger> m_log;

  };  // end JetReconstruction definition

}  // end eicrecon namespace

#endif
