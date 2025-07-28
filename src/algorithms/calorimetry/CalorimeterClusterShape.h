// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chao Peng, Dhevan Gangadharan, Sebouh Paul, Derek Anderson

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <algorithm>
#include <cmath>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "CalorimeterClusterShapeConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

// --------------------------------------------------------------------------
//! Algorithm input/output
// --------------------------------------------------------------------------
using CalorimeterClusterShapeAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::ClusterCollection,
                      std::optional<edm4eic::MCRecoClusterParticleAssociationCollection>>,
    algorithms::Output<edm4eic::ClusterCollection,
                       std::optional<edm4eic::MCRecoClusterParticleAssociationCollection>>>;

// --------------------------------------------------------------------------
//! Calculate cluster shapes for provided clusters
// --------------------------------------------------------------------------
/*! An algorithm which takes a collection of clusters,
   *  computes their cluster shape parameters, and saves
   *  outputs the same clusters with computed parameters.
   */
class CalorimeterClusterShape : public CalorimeterClusterShapeAlgorithm,
                                public WithPodConfig<CalorimeterClusterShapeConfig> {

public:
  // ctor
  CalorimeterClusterShape(std::string_view name)
      : CalorimeterClusterShapeAlgorithm{name,
                                         {"inputClusters", "inputMCClusterAssociations"},
                                         {"outputClusters", "outputMCClusterAssociations"},
                                         "Computes cluster shape parameters"} {}

  // public methods
  void init() final;
  void process(const Input&, const Output&) const final;

private:
  //! constant weighting
  static double constWeight(double /*E*/, double /*tE*/, double /*p*/, int /*type*/) { return 1.0; }

  //! linear weighting by energy
  static double linearWeight(double E, double /*tE*/, double /*p*/, int /*type*/) { return E; }

  //! log weighting by energy
  static double logWeight(double E, double tE, double base, int /*type*/) {
    return std::max(0., base + std::log(E / tE));
  }

  //! map of weighting method to function
  const std::map<std::string, std::function<double(double, double, double, int)>> m_weightMethods =
      {{"none", constWeight}, {"linear", linearWeight}, {"log", logWeight}};

  //! weight function selected by m_cfg.energyWeight
  std::function<double(double, double, double, int)> m_weightFunc;
};

} // namespace eicrecon
