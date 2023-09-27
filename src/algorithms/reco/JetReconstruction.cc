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

using namespace fastjet;



namespace eicrecon {

  void JetReconstruction::init(std::shared_ptr<spdlog::logger> logger) {

    // configure algorithm parameters
    u_rJet           = m_cfg.rJet;
    u_minCstPt       = m_cfg.minCstPt;
    u_maxCstPt       = m_cfg.maxCstPt;
    u_minJetPt       = m_cfg.minJetPt;
    u_ghostMaxRap    = m_cfg.ghostMaxRap;
    u_ghostArea      = m_cfg.ghostArea;
    u_numGhostRepeat = m_cfg.numGhostRepeat;
    u_jetAlgo        = m_cfg.jetAlgo;
    u_recombScheme   = m_cfg.recombScheme;
    u_areaType       = m_cfg.areaType;

    // if specified algorithm, recomb. scheme, or area type
    // are not defined, then issue warning and set it to
    // default values
    if (m_mapJetAlgo.find(u_jetAlgo) == m_mapJetAlgo.end()) {
      m_log->warn(" Unknown jet algorithm '{}' specified! Setting algorithm to default ({}) and proceeding.", u_jetAlgo, m_defaultFastjetOpts.jetAlgo);
      u_jetAlgo = m_defaultFastjetOpts.jetAlgo;
    }
    if (m_mapRecombScheme.find(u_recombScheme) == m_mapRecombScheme.end()) {
      m_log->warn(" Unknown recombination scheme '{}' specified! Setting scheme to default ({}) and proceeding.", u_recombScheme, m_defaultFastjetOpts.recombScheme);
      u_recombScheme = m_defaultFastjetOpts.recombScheme;
    }
    if (m_mapAreaType.find(u_areaType) == m_mapAreaType.end()) {
      m_log->warn(" Unknown area type '{}' specified! Setting type to default ({}) and proceeding.", u_areaType, m_defaultFastjetOpts.areaType);
      u_jetAlgo = m_defaultFastjetOpts.jetAlgo;
    }

    m_log = logger;
    m_log->trace("Initialized");
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
      if ((mom->pt() > u_minCstPt) && (mom->pt() < u_maxCstPt)) {
        particles.emplace_back(mom->px(), mom->py(), mom->pz(), mom->e());
      }
    }

    // Choose jet and area definitions
    JetDefinition jet_def(m_mapJetAlgo[u_jetAlgo], u_rJet);
    AreaDefinition area_def(m_mapAreaType[u_areaType], GhostedAreaSpec(u_ghostMaxRap, u_numGhostRepeat, u_ghostArea));

    // Run the clustering, extract the jets
    ClusterSequenceArea clus_seq(particles, jet_def, area_def);
    std::vector<PseudoJet> jets = sorted_by_pt(clus_seq.inclusive_jets(u_minJetPt));

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
