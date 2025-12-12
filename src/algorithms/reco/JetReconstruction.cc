// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson, Zhongling Ji, Dmitry Kalinkin, John Lajoie

// class definition
#include "JetReconstruction.h"

// for error handling
#include <edm4hep/MCParticleCollection.h> // IWYU pragma: keep
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fastjet/ClusterSequenceArea.hh>
#include <fastjet/GhostedAreaSpec.hh>
// for fastjet objects
#include <fastjet/PseudoJet.hh>
#include <fastjet/contrib/Centauro.hh>
#include <fmt/core.h>
#include <stdexcept>
#include <vector>

#include "algorithms/reco/JetReconstructionConfig.h"

using namespace fastjet;

namespace eicrecon {

template <typename InputT> void JetReconstruction<InputT>::init() {

  this->trace("Initialized");

  // if specified algorithm, recomb. scheme, or area type
  // are not defined, then issue error and throw exception
  try {
    m_mapJetAlgo.at(m_cfg.jetAlgo);
  } catch (std::out_of_range& out) {
    this->error(" Unknown jet algorithm \"{}\" specified!", m_cfg.jetAlgo);
    throw std::runtime_error(out.what());
  }

  try {
    m_mapRecombScheme.at(m_cfg.recombScheme);
  } catch (std::out_of_range& out) {
    this->error(" Unknown recombination scheme \"{}\" specified!", m_cfg.recombScheme);
    throw std::runtime_error(out.what());
  }

  try {
    m_mapAreaType.at(m_cfg.areaType);
  } catch (std::out_of_range& out) {
    this->error(" Unknown area type \"{}\" specified!", m_cfg.areaType);
    throw std::runtime_error(out.what());
  }

  // Choose jet definition based on no. of parameters
  switch (m_mapJetAlgo[m_cfg.jetAlgo]) {

  // contributed algorithms
  case JetAlgorithm::plugin_algorithm:

    // expand to other algorithms as required
    if (m_cfg.jetContribAlgo == "Centauro") {
      m_jet_plugin = std::make_unique<contrib::CentauroPlugin>(m_cfg.rJet);
      m_jet_def    = std::make_unique<JetDefinition>(m_jet_plugin.get());
    } else {
      this->error(" Unknown contributed FastJet algorithm \"{}\" specified!", m_cfg.jetContribAlgo);
      throw std::runtime_error("Invalid contributed FastJet algorithm");
    }
    break;

  // 0 parameter algorithms
  case JetAlgorithm::ee_kt_algorithm:
    m_jet_def = std::make_unique<JetDefinition>(m_mapJetAlgo[m_cfg.jetAlgo],
                                                m_mapRecombScheme[m_cfg.recombScheme]);
    break;

  // 2 parameter algorithms
  case JetAlgorithm::genkt_algorithm:
    [[fallthrough]];

  case JetAlgorithm::ee_genkt_algorithm:
    m_jet_def = std::make_unique<JetDefinition>(m_mapJetAlgo[m_cfg.jetAlgo], m_cfg.rJet, m_cfg.pJet,
                                                m_mapRecombScheme[m_cfg.recombScheme]);
    break;

  // all others have only 1 parameter
  default:
    m_jet_def = std::make_unique<JetDefinition>(m_mapJetAlgo[m_cfg.jetAlgo], m_cfg.rJet,
                                                m_mapRecombScheme[m_cfg.recombScheme]);
    break;

  } // end switch (jet algorithm)

  // Define jet area
  m_area_def = std::make_unique<AreaDefinition>(
      m_mapAreaType[m_cfg.areaType],
      GhostedAreaSpec(m_cfg.ghostMaxRap, m_cfg.numGhostRepeat, m_cfg.ghostArea));

} // end 'init()'

template <typename InputT>
void JetReconstruction<InputT>::process(
    const typename JetReconstructionAlgorithm<InputT>::Input& input,
    const typename JetReconstructionAlgorithm<InputT>::Output& output) const {
  // Grab input collections
  const auto [input_collection] = input;
  auto [jet_collection]         = output;

  // extract input momenta and collect into pseudojets
  std::vector<PseudoJet> particles;
  for (unsigned iInput = 0; const auto& input : *input_collection) {

    // get 4-vector
    const auto& momentum = input.getMomentum();
    const auto& energy   = input.getEnergy();
    const auto pt        = edm4hep::utils::magnitudeTransverse(momentum);

    // Only cluster particles within the given pt Range
    if ((pt > m_cfg.minCstPt) && (pt < m_cfg.maxCstPt)) {
      particles.emplace_back(momentum.x, momentum.y, momentum.z, energy);
      particles.back().set_user_index(iInput);
    }
    ++iInput;
  }

  // Skip empty
  if (particles.empty()) {
    this->trace("  Empty particle list.");
    return;
  }
  this->trace("  Number of particles: {}", particles.size());

  // Run the clustering, extract the jets
  fastjet::ClusterSequenceArea m_clus_seq(particles, *m_jet_def, *m_area_def);
  std::vector<PseudoJet> jets = sorted_by_pt(m_clus_seq.inclusive_jets(m_cfg.minJetPt));

  // Print out some infos
  this->trace("  Clustering with : {}", m_jet_def->description());

  // loop over jets
  for (unsigned i = 0; i < jets.size(); i++) {

    this->trace("  jet {}: pt = {}, y = {}, phi = {}", i, jets[i].pt(), jets[i].rap(),
                jets[i].phi());

    // create jet to store in output collection
    edm4eic::MutableReconstructedParticle jet_output = jet_collection->create();
    jet_output.setMomentum(edm4hep::Vector3f(jets[i].px(), jets[i].py(), jets[i].pz()));
    jet_output.setEnergy(jets[i].e());
    jet_output.setMass(jets[i].m());

    // link constituents to jet kinematic info
    std::vector<PseudoJet> csts = jets[i].constituents();
    for (const auto& cst : csts) {
      jet_output.addToParticles(input_collection->at(cst.user_index()));
    } // for constituent j
  } // for jet i

  // return the jets
} // end 'process(const T&)'

template class JetReconstruction<edm4eic::ReconstructedParticle>;

} // end namespace eicrecon
