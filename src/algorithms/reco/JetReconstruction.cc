// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Derek Anderson, Zhongling Ji

// class definition
#include "JetReconstruction.h"

// for error handling
#include <JANA/JException.h>
#include <Math/GenVector/LorentzVector.h>
#include <edm4hep/Vector3f.h>
#include <fastjet/ClusterSequenceArea.hh>
#include <fastjet/GhostedAreaSpec.hh>
// for fastjet objects
#include <fastjet/PseudoJet.hh>
#include <fmt/core.h>
#include <exception>
#include <stdexcept>

#include "algorithms/reco/JetReconstructionConfig.h"

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
  }



  std::unique_ptr<edm4eic::ReconstructedParticleCollection> JetReconstruction::process(
    const std::vector<const edm4hep::LorentzVectorE*> momenta) {

    // Store the jets
    std::unique_ptr<edm4eic::ReconstructedParticleCollection> jet_collection { std::make_unique<edm4eic::ReconstructedParticleCollection>() };

    // Skip empty
    if (momenta.empty()) {
      m_log->trace("  Empty particle list.");
      return jet_collection;
    }

    m_log->trace("  Number of particles: {}", momenta.size());

    // Particles for jet reconstrution
    std::vector<PseudoJet> particles;
    for (const auto &mom : momenta) {

      // Only cluster particles within the given pt Range
      if ((mom->pt() > m_cfg.minCstPt) && (mom->pt() < m_cfg.maxCstPt)) {
        particles.emplace_back(mom->px(), mom->py(), mom->pz(), mom->e());
      }
    }

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

      // Type = 0 for jets, Type = 1 for constituents
      // Use PDG values to match jets and constituents
      edm4eic::MutableReconstructedParticle jet_output = jet_collection->create();
      jet_output.setType(0);
      jet_output.setPDG(i);
      jet_output.setMomentum(edm4hep::Vector3f(jets[i].px(), jets[i].py(), jets[i].pz()));
      jet_output.setEnergy(jets[i].e());
      jet_output.setMass(jets[i].m());

      // loop over constituents
      std::vector<PseudoJet> csts = jets[i].constituents();
      for (unsigned j = 0; j < csts.size(); j++) {

        const double cst_pt = csts[j].pt();
        m_log->trace("    constituent {}'s pt: {}", j, cst_pt);

        // Type = 0 for jets, Type = 1 for constituents
        // Use PDG values to match jets and constituents
        edm4eic::MutableReconstructedParticle cst_output = jet_collection->create();
        cst_output.setType(1);
        cst_output.setPDG(i);
        cst_output.setMomentum(edm4hep::Vector3f(csts[j].px(), csts[j].py(), csts[j].pz()));
        cst_output.setEnergy(csts[j].e());
        cst_output.setMass(csts[j].m());
        //jet_output.addToParticles(cst_output);  // FIXME: global issue with podio reference
      } // for constituent j
    } // for jet i

    // return the jets
    return jet_collection;
  }

}  // end namespace eicrecon
