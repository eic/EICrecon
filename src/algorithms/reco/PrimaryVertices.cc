// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Daniel Brandenburg, Xin Dong

#include <edm4eic/VertexCollection.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/Vector4f.h>
#include <podio/RelationRange.h>
#include <cmath>
#include <functional>
#include <gsl/pointers>
#include <map>
#include <utility>

#include "algorithms/reco/PrimaryVertices.h"
#include "algorithms/reco/PrimaryVerticesConfig.h"

namespace eicrecon {

/**
   * @brief Initialize the PrimaryVertices Algorithm
   *
   */
void PrimaryVertices::init() {}

/**
   * @brief Produce a list of primary vertex candidates
   *
   * @param rcvtx  - input collection of all vertex candidates
   * @return edm4eic::VertexCollection
   */
void PrimaryVertices::process(const PrimaryVertices::Input& input,
                              const PrimaryVertices::Output& output) const {
  const auto [rcvtx]          = input;
  auto [out_primary_vertices] = output;

  // this multimap will store intermediate results
  // so that we can sort them before filling output
  // collection
  std::multimap<int, edm4eic::Vertex, std::greater<>> primaryVertexMap;

  // our output collection of primary vertex
  // ordered by N_trk = associatedParticle array size
  out_primary_vertices->setSubsetCollection();

  trace("We have {} candidate vertices", rcvtx->size());

  for (const auto& vtx : *rcvtx) {
    const auto& v = vtx.getPosition();

    // some basic vertex selection
    if (sqrt(v.x * v.x + v.y * v.y) / edm4eic::unit::mm > m_cfg.maxVr ||
        std::abs(v.z) / edm4eic::unit::mm > m_cfg.maxVz) {
      continue;
    }

    if (vtx.getChi2() > m_cfg.maxChi2) {
      continue;
    }

    int N_trk = vtx.getAssociatedParticles().size();
    trace("\t N_trk = {}", N_trk);
    primaryVertexMap.insert({N_trk, vtx});
  } // vertex loop

  bool first = true;
  // map defined with std::greater<> will be iterated in descending order by the key
  for (auto [N_trk, vertex] : primaryVertexMap) {
    // Do not save primary candidates that
    // are not within range
    if (N_trk > m_cfg.maxNtrk || N_trk < m_cfg.minNtrk) {
      continue;
    }

    // For logging and development
    // report the highest N_trk candidate chosen
    if (first) {
      trace("Max N_trk Candidate:");
      trace("\t N_trk = {}", N_trk);
      trace("\t Primary vertex has xyz=( {}, {}, {} )", vertex.getPosition().x,
            vertex.getPosition().y, vertex.getPosition().z);
      first = false;
    }
    out_primary_vertices->push_back(vertex);
  } // loop on primaryVertexMap
}
} // namespace eicrecon
