// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Derek Anderson, Zhongling Ji

// class definition
#include "JetReconstruction.h"

// c utilities
#include <cmath>
// event data model related classes
#include <edm4eic/vector_utils.h>
#include <edm4eic/MutableReconstructedParticle.h>
// for fastjet objects
#include <fastjet/PseudoJet.hh>
#include <fastjet/ClusterSequenceArea.hh>
// for error handling
#include <JANA/JException.h>

using namespace fastjet;

namespace eicrecon {

  void JetReconstruction::init(std::shared_ptr<spdlog::logger> logger) {

    m_log = logger;
    m_log->trace("Initialized");

    // if specified algorithm, recomb. scheme, or area type
    // are not defined, then issue error and throw exception
    try {
      m_mapJetAlgo.at(m_cfg.jetAlgo);
    } catch (std::out_of_range &out) {
      m_log->error(" Unknown jet algorithm \"{}\" specified!", m_cfg.jetAlgo);
      throw JException(out.what());
    }

    try {
      m_mapRecombScheme.at(m_cfg.recombScheme);
    } catch (std::out_of_range &out) {
      m_log->error(" Unknown recombination scheme \"{}\" specified!", m_cfg.recombScheme);
      throw JException(out.what());
    }

    try {
      m_mapAreaType.at(m_cfg.areaType);
    } catch (std::out_of_range &out) {
      m_log->error(" Unknown area type \"{}\" specified!", m_cfg.areaType);
      throw JException(out.what());
    }
  }  // end 'init(std::shared_ptr<spdlog::logger>)'



  template <typename T> std::unique_ptr<edm4eic::ReconstructedParticleCollection> JetReconstruction::process(const T& input_collection) {

    // Store the jets
    std::unique_ptr<edm4eic::ReconstructedParticleCollection> jet_collection { std::make_unique<edm4eic::ReconstructedParticleCollection>() };

    // extract input momenta and collect into pseudojets
    std::vector<PseudoJet> particles;
    for (unsigned iInput = 0; const auto& input : *input_collection) {

      // get 4-vector
      const auto& momentum = input.getMomentum();
      const auto& energy = input.getEnergy();
      const auto* lorentz = new edm4hep::LorentzVectorE(momentum.x, momentum.y, momentum.z, energy);

      // Only cluster particles within the given pt Range
      if ((lorentz->pt() > m_cfg.minCstPt) && (lorentz->pt() < m_cfg.maxCstPt)) {
        particles.emplace_back(lorentz->px(), lorentz->py(), lorentz->pz(), lorentz->e());
        particles.back().set_user_index(iInput);
      }
      ++iInput;
    }

    // Skip empty
    if (particles.empty()) {
      m_log->trace("  Empty particle list.");
      return jet_collection;
    }
    m_log->trace("  Number of particles: {}", particles.size());

    // Choose jet and area definitions
    JetDefinition jet_def(m_mapJetAlgo[m_cfg.jetAlgo], m_cfg.rJet, m_mapRecombScheme[m_cfg.recombScheme]);
    AreaDefinition area_def(m_mapAreaType[m_cfg.areaType], GhostedAreaSpec(m_cfg.ghostMaxRap, m_cfg.numGhostRepeat, m_cfg.ghostArea));

    // Run the clustering, extract the jets
    ClusterSequenceArea clus_seq(particles, jet_def, area_def);
    std::vector<PseudoJet> jets = sorted_by_pt(clus_seq.inclusive_jets(m_cfg.minJetPt));

    // Print out some infos
    m_log->trace("  Clustering with : {}", jet_def.description());

    // loop over jets
    for (unsigned i = 0; i < jets.size(); i++) {

      m_log->trace("  jet {}: pt = {}, y = {}, phi = {}", i, jets[i].pt(), jets[i].rap(), jets[i].phi());

      // create jet to store in output collection
      edm4eic::MutableReconstructedParticle jet_output = jet_collection->create();
      jet_output.setMomentum(edm4hep::Vector3f(jets[i].px(), jets[i].py(), jets[i].pz()));
      jet_output.setEnergy(jets[i].e());
      jet_output.setMass(jets[i].m());

      // link constituents to jet kinematic info
      std::vector<PseudoJet> csts = jets[i].constituents();
      for (unsigned j = 0; j < csts.size(); j++) {
        add_to_jet_kinematics(input_collection[csts[j].user_index()], jet_output);
      } // for constituent j
    } // for jet i

    // return the jets
    return jet_collection;
  }  // end 'process(const T&)'

}  // end namespace eicrecon
