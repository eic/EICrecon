// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Derek Anderson, Zhongling Ji

#ifndef EICRECON_JETRECONSTRUCTION_H
#define EICRECON_JETRECONSTRUCTION_H

#include <map>
#include <vector>
#include <string>
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

      // configurable  parameters
      float       u_rJet;            // jet resolution  parameter
      double      u_minCstPt;        // minimum pT of objects fed to cluster sequence
      double      u_maxCstPt;        // maximum pT of objects fed to clsuter sequence
      double      u_minJetPt;        // minimum jet pT
      double      u_ghostMaxRap;     // maximum rapidity of ghosts
      double      u_ghostArea;       // area per ghost
      int         u_numGhostRepeat;  // number of times a ghost is reused per grid site
      std::string u_jetAlgo;         // jet finding algorithm
      std::string u_recombScheme;    // particle recombination scheme
      std::string u_areaType;        // type of area calculated

      // logger and configuration
      std::shared_ptr<spdlog::logger> m_log;
      JetReconstructionConfig         m_cfg;

      // maps of user input onto fastjet options
      const std::map<std::string, fastjet::JetAlgorithm> m_mapJetAlgo = {
        {"kt_algorithm",                    fastjet::JetAlgorithm::kt_algorithm},
        {"cambridge_algorithm",             fastjet::JetAlgorithm::cambridge_algorithm},
        {"antikt_algorithm",                fastjet::JetAlgorithm::antikt_algorithm},
        {"genkt_algorithm",                 fastjet::JetAlgorithm::genkt_algorithm},
        {"cambridge_for_passive_algorithm", fastjet::JetAlgorithm::cambridge_for_passive_algorithm},
        {"genkt_for_passive_algorithm",     fastjet::JetAlgorithm::genkt_for_passive_algorithm},
        {"ee_kt_algorithm",                 fastjet::JetAlgorithm::ee_kt_algorithm},
        {"ee_genkt_algorithm",              fastjet::JetAlgorithm::ee_genkt_algorithm},
        {"plugin_algorithm",                fastjet::JetAlgorithm::plugin_algorithm}
      };
      const std::map<std::string, fastjet::RecombinationScheme> m_mapRecombScheme = {
        {"E_scheme",        fastjet::RecombinationScheme::E_scheme},
        {"pt_scheme",       fastjet::RecombinationScheme::pt_scheme},
        {"pt2_scheme",      fastjet::RecombinationScheme::pt2_scheme},
        {"Et_scheme",       fastjet::RecombinationScheme::Et_scheme},
        {"Et2_scheme",      fastjet::RecombinationScheme::Et2_scheme},
        {"BIpt_scheme",     fastjet::RecombinationScheme::BIpt_scheme},
        {"BIpt2_scheme",    fastjet::RecombinationScheme::BIpt2_scheme},
        {"WTA_pt_scheme",   fastjet::RecombinationScheme::WTA_pt_scheme},
        {"WTA_modp_scheme", fastjet::RecombinationScheme::WTA_modp_scheme},
        {"external_scheme", fastjet::RecombinationScheme::external_scheme}
      };
      const std::map<std::string, fastjet::AreaType> m_mapAreaType = {
        {"active_area",                 fastjet::AreaType::active_area},
        {"active_area_explicit_ghosts", fastjet::AreaType::active_area_explicit_ghosts},
        {"one_ghost_passive_area",      fastjet::AreaType::one_ghost_passive_area},
        {"passive_area",                fastjet::AreaType::passive_area},
        {"voronoi_area",                fastjet::AreaType::voronoi_area}
      };

      // default fastjet options
      const struct defaults {
        std::string jetAlgo;
        std::string recombScheme;
        std::string areaType;
      } m_defaultFastjetOpts = {"antikt_algorithm", "E_scheme", "active_area"};

  };  // end JetReconstruction definition

}  // end eicrecon namespace

#endif
