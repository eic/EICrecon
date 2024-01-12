// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Derek Anderson, Zhongling Ji

#ifndef EICRECON_JETRECONSTRUCTION_H
#define EICRECON_JETRECONSTRUCTION_H

#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticle.h>
#include <edm4hep/utils/kinematics.h>
#include <fastjet/AreaDefinition.hh>
#include <fastjet/JetDefinition.hh>
#include <spdlog/logger.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "JetReconstructionConfig.h"
// for algorithm configuration
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

  class JetReconstruction : public WithPodConfig<JetReconstructionConfig> {

    public:

      // algorithm initialization
      void init(std::shared_ptr<spdlog::logger> logger);

      // run algorithm
      template<typename T> std::unique_ptr<edm4eic::ReconstructedParticleCollection> process(const T* input_collection);

    private:

      // generic method to add constituent to jet kinematic info
      template <typename T> void add_to_jet_kinematics(const T& addend, edm4eic::MutableReconstructedParticle& kinematics, edm4eic::ReconstructedParticleCollection*);

      // specialization for ReconstructedParticle input
      void add_to_jet_kinematics(const edm4eic::ReconstructedParticle& addend, edm4eic::MutableReconstructedParticle& kinematics, edm4eic::ReconstructedParticleCollection*) {
        kinematics.addToParticles(addend);
      };

      // specialization for MCParticle input
      void add_to_jet_kinematics(const edm4hep::MCParticle& addend, edm4eic::MutableReconstructedParticle& kinematics, edm4eic::ReconstructedParticleCollection *output_coll) {
        kinematics.addToParticles(copy_mcparticle_onto_recoparticle(addend, output_coll));
      };

      // helper function to copy edm4hep::MCParticle onto edm4eic::ReconstructedParticle
      edm4eic::ReconstructedParticle copy_mcparticle_onto_recoparticle(const edm4hep::MCParticle& mc, edm4eic::ReconstructedParticleCollection *output_coll) {
        const float mom = std::hypot(mc.getMomentum().x, mc.getMomentum().y, mc.getMomentum().z);
        const float energy = std::hypot(mc.getMass(), mom);

        // create reco particle
        edm4eic::MutableReconstructedParticle mutable_reco = output_coll->create();
        mutable_reco.setMomentum(mc.getMomentum());
        mutable_reco.setEnergy(energy);
        mutable_reco.setCharge(mc.getCharge());
        mutable_reco.setMass(mc.getMass());
        mutable_reco.setPDG(mc.getPDG());

        // return non-mutable representation
        return static_cast<edm4eic::ReconstructedParticle>(mutable_reco);
      }

      std::shared_ptr<spdlog::logger> m_log;

      // maps of user input onto fastjet options
      std::map<std::string, fastjet::JetAlgorithm> m_mapJetAlgo = {
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
      std::map<std::string, fastjet::RecombinationScheme> m_mapRecombScheme = {
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
      std::map<std::string, fastjet::AreaType> m_mapAreaType = {
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
