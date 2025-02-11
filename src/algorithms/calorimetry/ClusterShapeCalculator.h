// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 <OG PEOPLE>, Derek Anderson 

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"



namespace eicrecon {

  // --------------------------------------------------------------------------
  //! Algorithm input/output
  // --------------------------------------------------------------------------
  using ClusterShapeCalculatorAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      edm4eic::ClusterCollection,
      std::optional<edm4eic::MCRecoClusterParticleAssociationCollection>
    >,
    algorithms::Output<
      edm4eic::ClusterCollection,
      std::optional<edm4eic::MCRecoClusterParticleAssociationCollection>
    >
  >;



  // --------------------------------------------------------------------------
  //! Calculate cluster shapes for provided clusters
  // --------------------------------------------------------------------------
  /*! An algorithm which takes a collection of clusters,
   *  computes their cluster shape parameters, and saves
   *  outputs the same clusters with computed parameters.
   */ 
  class ClusterShapeCalculator
    : public ClusterShapeCalculatorAlgorithm
    , public WithPodConfig<NoConfig> 
  {

    public:

      // ctor
      ClusterShapeCalculator(std::string_view name) :
        ClusterShapeCalculatorAlgorithm {
          name,
          {"InputClusters", "InputMCClusterAssociations"},
          {"OutputClusters", "OutputMCClusterAssociations"},
          "Computes cluster shape parameters."
        } {}

      // public methods
      void init() final;
      void process(const Input&, const Output&) const final;

  };

}  // namespace eicrecon
