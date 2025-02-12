// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chao Peng, Dhevan Gangadharan, Sebouh Paul, Derek Anderson

#pragma once

#include "algorithms/interfaces/WithPodConfig.h"
#include "ClusterShapeCalculatorConfig.h"

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <optional>
#include <string>
#include <string_view>



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
    , public WithPodConfig<ClusterShapeCalculatorConfig>
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
