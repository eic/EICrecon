// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Daniel Brandenburg, Xin Dong

#include <DD4hep/Fields.h>
#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <edm4eic/VertexCollection.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/Vector4f.h>
#include <edm4hep/utils/vector_utils.h>
#include <cmath>
#include <tuple>
#include <utility>
#include <vector>

#include "algorithms/reco/Helix.h"
#include "algorithms/reco/SecondaryVerticesHelix.h"
#include "algorithms/reco/SecondaryVerticesHelixConfig.h"
#include "services/particle/ParticleSvc.h"

namespace eicrecon {

/**
   * @brief Initialize the SecondaryVerticesHelix Algorithm
   *
   */
void SecondaryVerticesHelix::init() {}

/**
   * @brief Produce a list of secondary vertex candidates
   *
   * @param rcvtx  - input collection of all vertex candidates
   * @return edm4eic::VertexCollection
   */
void SecondaryVerticesHelix::process(const SecondaryVerticesHelix::Input& input,
                                     const SecondaryVerticesHelix::Output& output) const {
  const auto [rcvtx, rcparts_collections] = input;
  auto [out_secondary_vertices]           = output;

  auto& particleSvc = algorithms::ParticleSvc::instance();

  if (rcvtx->empty()) {
    debug("No primary vertex in this event");
    return;
  }
  const auto pVtxPos4f = (*rcvtx)[0].getPosition();
  // convert to cm
  edm4hep::Vector3f pVtxPos(pVtxPos4f.x * edm4eic::unit::mm / edm4eic::unit::cm,
                            pVtxPos4f.y * edm4eic::unit::mm / edm4eic::unit::cm,
                            pVtxPos4f.z * edm4eic::unit::mm / edm4eic::unit::cm);

  auto fieldObj = m_det->field();
  auto field    = fieldObj.magneticField(
      {pVtxPos4f.x / edm4eic::unit::mm * dd4hep::mm, pVtxPos4f.y / edm4eic::unit::mm * dd4hep::mm,
       pVtxPos4f.z / edm4eic::unit::mm * dd4hep::mm}); // in unit of dd4hep::tesla
  float b_field = field.z();

  debug("Primary vertex = ({},{},{})cm \t b field = {} tesla", pVtxPos.x, pVtxPos.y, pVtxPos.z,
        b_field / dd4hep::tesla);

  struct TrackCandidate {
    Helix helix;
    edm4eic::ReconstructedParticle particle;
    float field;
  };

  std::vector<TrackCandidate> candidates;
  candidates.clear();

  for (std::size_t i_coll = 0; i_coll < rcparts_collections.size(); ++i_coll) {
    const auto& rcparts    = rcparts_collections[i_coll];
    const float trackField = (i_coll == 0) ? b_field : 0;
    for (const auto& p : *rcparts) {
      if (p.getCharge() == 0) {
        continue;
      }
      Helix h(p, trackField);
      double dca = h.distance(pVtxPos) * edm4eic::unit::cm;
      if (dca < m_cfg.minDca) {
        continue;
      }
      candidates.push_back({h, p, trackField});
    }
  }

  debug("\tVector size {}", candidates.size());

  for (unsigned int i1 = 0; i1 < candidates.size(); ++i1) {
    for (unsigned int i2 = i1 + 1; i2 < candidates.size(); ++i2) {
      const auto& p1 = candidates[i1].particle;
      const auto& p2 = candidates[i2].particle;

      if (!(m_cfg.unlikesign && p1.getCharge() + p2.getCharge() == 0)) {
        continue;
}

      const auto& h1 = candidates[i1].helix;
      const auto& h2 = candidates[i2].helix;

      // Helix function uses cm unit
      double dca1 = h1.distance(pVtxPos) * edm4eic::unit::cm;
      double dca2 = h2.distance(pVtxPos) * edm4eic::unit::cm;
      if (dca1 < m_cfg.minDca || dca2 < m_cfg.minDca)
        continue;

      std::pair<double, double> const ss = h1.pathLengths(h2);
      edm4hep::Vector3f h1AtDcaTo2       = h1.at(ss.first);
      edm4hep::Vector3f h2AtDcaTo1       = h2.at(ss.second);

      double dca12 = edm4hep::utils::magnitude(h1AtDcaTo2 - h2AtDcaTo1) * edm4eic::unit::cm;
      if (std::isnan(dca12))
        continue;
      if (dca12 > m_cfg.maxDca12)
        continue;
      edm4hep::Vector3f pairPos = 0.5 * (h1AtDcaTo2 + h2AtDcaTo1);

      edm4hep::Vector3f h1MomAtDca = h1.momentumAt(ss.first, candidates[i1].field);
      edm4hep::Vector3f h2MomAtDca = h2.momentumAt(ss.second, candidates[i2].field);
      edm4hep::Vector3f pairMom    = h1MomAtDca + h2MomAtDca;

      double e1 =
          std::hypot(edm4hep::utils::magnitude(h1MomAtDca), particleSvc.particle(p1.getPDG()).mass);
      double e2 =
          std::hypot(edm4hep::utils::magnitude(h2MomAtDca), particleSvc.particle(p2.getPDG()).mass);
      double pairE = e1 + e2;
      double angle = edm4hep::utils::angleBetween(pairMom, pairPos - pVtxPos);
      if (cos(angle) < m_cfg.minCostheta)
        continue;

      double beta = edm4hep::utils::magnitude(pairMom) / pairE;
      double time = edm4hep::utils::magnitude(pairPos - pVtxPos) / (beta * dd4hep::c_light);
      edm4hep::Vector3f dL = pairPos - pVtxPos; // in cm
      edm4hep::Vector3f decayL(dL.x * edm4eic::unit::cm, dL.y * edm4eic::unit::cm,
                               dL.z * edm4eic::unit::cm);
      double dca2pv = edm4hep::utils::magnitude(decayL) * sin(angle);
      if (dca2pv > m_cfg.maxDca)
        continue;

      auto v0 = out_secondary_vertices->create();
      v0.setType(2); // 2 for secondary
      v0.setPosition({(float)(pairPos.x * edm4eic::unit::cm / edm4eic::unit::mm),
                      (float)(pairPos.y * edm4eic::unit::cm / edm4eic::unit::mm),
                      (float)(pairPos.z * edm4eic::unit::cm / edm4eic::unit::mm), (float)time});
      v0.addToAssociatedParticles(p1);
      v0.addToAssociatedParticles(p2);

      debug("One secondary vertex found at (x,y,z) = ({}, {}, {}) mm.",
            pairPos.x * edm4eic::unit::cm / edm4eic::unit::mm,
            pairPos.y * edm4eic::unit::cm / edm4eic::unit::mm,
            pairPos.z * edm4eic::unit::cm / edm4eic::unit::mm);

    } // end i2
  } // end i1

} // end process

} // namespace eicrecon
