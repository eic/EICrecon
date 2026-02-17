// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson, Zhongling Ji, Dmitry Kalinkin, John Lajoie

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/EventHeaderCollection.h>
#include <fastjet/AreaDefinition.hh>
#include <fastjet/JetDefinition.hh>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "JetReconstructionConfig.h"
// for algorithm configuration
#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/interfaces/UniqueIDGenSvc.h"

namespace eicrecon {

template <typename InputT>
using JetReconstructionAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::EventHeaderCollection, typename InputT::collection_type>,
    algorithms::Output<edm4eic::ReconstructedParticleCollection>>;

template <typename InputT>
class JetReconstruction : public JetReconstructionAlgorithm<InputT>,
                          public WithPodConfig<JetReconstructionConfig> {

public:
  JetReconstruction(std::string_view name)
      : JetReconstructionAlgorithm<InputT>{
            name,
            {"eventHeaderCollection", "inputReconstructedParticles"},
            {"outputReconstructedParticles"},
            "Performs jet reconstruction using a FastJet algorithm."} {}

public:
  // algorithm initialization
  void init() final;

  // run algorithm
  void process(const typename eicrecon::JetReconstructionAlgorithm<InputT>::Input&,
               const typename eicrecon::JetReconstructionAlgorithm<InputT>::Output&) const final;

private:
  // fastjet components
  std::unique_ptr<fastjet::JetDefinition> m_jet_def;
  std::unique_ptr<fastjet::AreaDefinition> m_area_def;
  std::unique_ptr<fastjet::JetDefinition::Plugin> m_jet_plugin;

  // maps of user input onto fastjet options
  std::map<std::string, fastjet::JetAlgorithm> m_mapJetAlgo = {
      {"kt_algorithm", fastjet::JetAlgorithm::kt_algorithm},
      {"cambridge_algorithm", fastjet::JetAlgorithm::cambridge_algorithm},
      {"antikt_algorithm", fastjet::JetAlgorithm::antikt_algorithm},
      {"genkt_algorithm", fastjet::JetAlgorithm::genkt_algorithm},
      {"cambridge_for_passive_algorithm", fastjet::JetAlgorithm::cambridge_for_passive_algorithm},
      {"genkt_for_passive_algorithm", fastjet::JetAlgorithm::genkt_for_passive_algorithm},
      {"ee_kt_algorithm", fastjet::JetAlgorithm::ee_kt_algorithm},
      {"ee_genkt_algorithm", fastjet::JetAlgorithm::ee_genkt_algorithm},
      {"plugin_algorithm", fastjet::JetAlgorithm::plugin_algorithm}};
  std::map<std::string, fastjet::RecombinationScheme> m_mapRecombScheme = {
      {"E_scheme", fastjet::RecombinationScheme::E_scheme},
      {"pt_scheme", fastjet::RecombinationScheme::pt_scheme},
      {"pt2_scheme", fastjet::RecombinationScheme::pt2_scheme},
      {"Et_scheme", fastjet::RecombinationScheme::Et_scheme},
      {"Et2_scheme", fastjet::RecombinationScheme::Et2_scheme},
      {"BIpt_scheme", fastjet::RecombinationScheme::BIpt_scheme},
      {"BIpt2_scheme", fastjet::RecombinationScheme::BIpt2_scheme},
      {"WTA_pt_scheme", fastjet::RecombinationScheme::WTA_pt_scheme},
      {"WTA_modp_scheme", fastjet::RecombinationScheme::WTA_modp_scheme},
      {"external_scheme", fastjet::RecombinationScheme::external_scheme}};
  std::map<std::string, fastjet::AreaType> m_mapAreaType = {
      {"active_area", fastjet::AreaType::active_area},
      {"active_area_explicit_ghosts", fastjet::AreaType::active_area_explicit_ghosts},
      {"one_ghost_passive_area", fastjet::AreaType::one_ghost_passive_area},
      {"passive_area", fastjet::AreaType::passive_area},
      {"voronoi_area", fastjet::AreaType::voronoi_area}};

  // default fastjet options
  const struct defaults {
    std::string jetAlgo;
    std::string recombScheme;
    std::string areaType;
  } m_defaultFastjetOpts = {"antikt_algorithm", "E_scheme", "active_area"};

  // unique ID service for generating reproducible seeds
  const algorithms::UniqueIDGenSvc& m_uid = algorithms::UniqueIDGenSvc::instance();

}; // end JetReconstruction definition

} // namespace eicrecon
