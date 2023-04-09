// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Derek Anderson, Zhongling Ji

// class definition
#include "JetReconstruction.h"

// standard c includes
#include <cmath>
// Event Model related classes
#include <edm4eic/vector_utils.h>
#include <edm4eic/MutableReconstructedParticle.h>
// fastjet includes
#include <fastjet/PseudoJet.hh>
#include <fastjet/ClusterSequenceArea.hh>

using namespace fastjet;


namespace eicrecon {

  void JetReconstruction::init(std::shared_ptr<spdlog::logger> logger) {
    m_log = logger;
    m_log->trace("Initialized");
  }

  std::vector<edm4eic::ReconstructedParticle*> JetReconstruction::execute(
    const std::vector<const edm4hep::LorentzVectorE*> momenta) {

    // Store the jets
    std::vector<edm4eic::ReconstructedParticle*> jets_edm;

    // Skip empty
    if (momenta.empty()) {
      m_log->trace("  Empty particle list.");
      return jets_edm;
    }

    m_log->trace("  Number of particles: {}", momenta.size());

    // Particles for jet reconstrution
    std::vector<PseudoJet> particles;
    for (const auto &mom : momenta) {
      double partPt = std::sqrt(mom->px()*mom->px() + mom->py()*mom->py());
      if(partPt > m_minCstPt && partPt < m_maxCstPt) // Only cluster particles within the given pt Range
        particles.push_back( PseudoJet(mom->px(), mom->py(), mom->pz(), mom->e()) );
    }

    // Choose jet and area definitions
    JetDefinition jet_def(m_jetAlgo, m_rJet);
    AreaDefinition area_def(m_areaType, GhostedAreaSpec(m_ghostMaxRap, m_numGhostRepeat, m_ghostArea));

    // Run the clustering, extract the jets
    ClusterSequenceArea clus_seq(particles, jet_def, area_def);
    std::vector<PseudoJet> jets = sorted_by_pt(clus_seq.inclusive_jets());

    // Print out some infos
    m_log->trace("  Clustering with : {}", jet_def.description());

    for (unsigned i = 0; i < jets.size(); i++) {

      m_log->trace("  jet {}: pt = {}, y = {}, phi = {}", i, jets[i].pt(), jets[i].rap(), jets[i].phi());
      edm4eic::MutableReconstructedParticle jet_edm;
      // Type = 0 for jets, Type = 1 for constituents
      // Use PDG values to match jets and constituents
      jet_edm.setType(0);
      jet_edm.setPDG(i);
      jet_edm.setMomentum(edm4hep::Vector3f(jets[i].px(), jets[i].py(), jets[i].pz()));
      jet_edm.setEnergy(jets[i].e());
      jet_edm.setMass(jets[i].m());

      std::vector<PseudoJet> csts = jets[i].constituents();
      for (unsigned j = 0; j < csts.size(); j++) {
        const double cst_pt = csts[j].pt();
        m_log->trace("    constituent {}'s pt: {}", j, cst_pt);

        edm4eic::MutableReconstructedParticle cst_edm;
        // Type = 0 for jets, Type = 1 for constituents
        // Use PDG values to match jets and constituents
        cst_edm.setType(1);
        cst_edm.setPDG(i);
        cst_edm.setMomentum(edm4hep::Vector3f(csts[j].px(), csts[j].py(), csts[j].pz()));
        cst_edm.setEnergy(csts[j].e());
        cst_edm.setMass(csts[j].m());
        //jet_edm.addToParticles(cst_edm);  // FIXME: global issue with podio reference
        // Store constituents in jets due to the above issue
        jets_edm.push_back(new edm4eic::ReconstructedParticle(cst_edm));
      } // for constituent j

      jets_edm.push_back(new edm4eic::ReconstructedParticle(jet_edm));
    } // for jet i

    // return the jets
    return jets_edm;
  }

} // namespace eicrecon
