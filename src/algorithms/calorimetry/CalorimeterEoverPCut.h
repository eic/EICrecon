// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 You

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4hep/ParticleIDCollection.h>

namespace eicrecon {

// Base alias: inputs=(clusters + optional assocs), outputs=ParticleIDCollection
using CalorimeterEoverPCutAlgorithmBase = algorithms::Algorithm<
    algorithms::Input<edm4eic::ClusterCollection,
                      std::optional<edm4eic::MCRecoClusterParticleAssociationCollection>>,
    algorithms::Output<edm4hep::ParticleIDCollection>>;

/// A simple E/P‑cut algorithm holding its own threshold and layer limit
class CalorimeterEoverPCut : public CalorimeterEoverPCutAlgorithmBase {
public:
  CalorimeterEoverPCut(std::string_view name)
      : CalorimeterEoverPCutAlgorithmBase{name,
                                          {"inputClusters", "inputAssocs"},
                                          {"outputPIDs"},
                                          "E/P Cut (manual config)"}
      , m_ecut(0.74)
      , m_maxLayer(12) {}

  void init() final {} // nothing to do at run start
  void process(const Input& input, const Output& output) const final;

  // setters for factory to override defaults:
  void setEcut(double e) { m_ecut = e; }
  void setMaxLayer(int maxL) { m_maxLayer = maxL; }

private:
  double m_ecut;  ///< E/P threshold
  int m_maxLayer; ///< (unused) integration depth
};

} // namespace eicrecon
