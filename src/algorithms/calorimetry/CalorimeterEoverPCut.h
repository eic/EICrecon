// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4hep/ParticleIDCollection.h>

namespace eicrecon {

using CalorimeterEoverPCutAlgorithmBase = algorithms::Algorithm<
    algorithms::Input<
      edm4eic::ClusterCollection,
      edm4eic::MCRecoClusterParticleAssociationCollection,
      edm4eic::CalorimeterHitCollection
    >,
    algorithms::Output<edm4hep::ParticleIDCollection>>;

class CalorimeterEoverPCut : public CalorimeterEoverPCutAlgorithmBase {
public:
  CalorimeterEoverPCut(std::string_view name)
      : CalorimeterEoverPCutAlgorithmBase{name,
                                          {"inputClusters", "inputAssocs", "inputHits"},
                                          {"outputPIDs"},
                                          "E/P Cut with layer‐depth limit"}
      , m_ecut(0.74)
      , m_maxLayer(12) {}

  void init() final {} // nothing to do at run start
  void process(const Input& input, const Output& output) const final;

  void setEcut(double e) { m_ecut = e; }
  void setMaxLayer(int maxL) { m_maxLayer = maxL; }

private:
  double m_ecut;  // E/P threshold
  int m_maxLayer; // integration depth
};

} // namespace eicrecon
