// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Daniel Brandenburg, Xin Dong

#include <edm4eic/Track.h>
#include <edm4eic/Trajectory.h>
#include <edm4eic/VertexCollection.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/Vector4f.h>
#include <fmt/core.h>
#include <podio/RelationRange.h>
#include <cmath>
#include <functional>
#include <gsl/pointers>
#include <map>
#include <utility>
#include <vector>

#include "algorithms/reco/SecondaryVerticesHelix.h"
#include "algorithms/reco/Helix.h"
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
  const auto [rcvtx, rcparts] = input;
  auto [out_secondary_vertices] = output;

  auto& particleSvc = algorithms::ParticleSvc::instance();
//  edm4hep::Vector3f pVtxPos;
//  for(const auto& v : primVtx)
  const auto pVtxPos4f = (*rcvtx)[0].getPosition();
  // convert to cm
  edm4hep::Vector3f pVtxPos(pVtxPos4f.x*edm4eic::unit::mm/edm4eic::unit::cm, 
                            pVtxPos4f.y*edm4eic::unit::mm/edm4eic::unit::cm, 
                            pVtxPos4f.z*edm4eic::unit::mm/edm4eic::unit::cm);
  info("\t Primary vertex = ({},{},{})cm \t b field = {} tesla", pVtxPos.x, pVtxPos.y, pVtxPos.z, m_cfg.b_field/dd4hep::tesla);
  
  std::vector<unsigned int> pi_index;
  std::vector<unsigned int> k_index;
  std::vector<unsigned int> p_index;
  for (unsigned int i = 0; const auto& p : *rcparts) {
    const auto pdg = p.getPDG();
    if(abs(pdg) == 211) pi_index.push_back(i);
    if(abs(pdg) == 321) k_index.push_back(i);
    if(abs(pdg) == 2212) p_index.push_back(i);
    ++i;  
  }
  
  info("\t Array sizes: pions  = {}, kaons = {}, protons = {}", pi_index.size(), k_index.size(), p_index.size());
    
  for (unsigned int i1 = 0; i1 < pi_index.size(); ++i1) {
    for (unsigned int i2 = i1 + 1; i2 < pi_index.size(); ++i2) {
       const auto& p1 = (*rcparts)[i1];
       const auto& p2 = (*rcparts)[i2];
       
       if (p1.getCharge() + p2.getCharge() != 0) continue;
       
       Helix h1obj(p1, m_cfg.b_field);  Helix& h1 = h1obj;
       Helix h2obj(p2, m_cfg.b_field);  Helix& h2 = h2obj;
       
       // Helix function uses cm unit
       double dca1 = h1.distance(pVtxPos) * edm4eic::unit::cm;
       double dca2 = h2.distance(pVtxPos) * edm4eic::unit::cm;
       debug("\t dca1 = {}, dca2 = {}", dca1, dca2);
       if( dca1 < m_cfg.minDca1 || dca2 < m_cfg.minDca2 ) continue;
       
       std::pair<double, double> const ss = h1.pathLengths(h2);
       edm4hep::Vector3f h1AtDcaTo2 = h1.at(ss.first);
       edm4hep::Vector3f h2AtDcaTo1 = h2.at(ss.second);
       
       double dca12 = edm4hep::utils::magnitude(h1AtDcaTo2 - h2AtDcaTo1) * edm4eic::unit::cm;
       if( dca12 > m_cfg.maxDca12 ) continue;
       edm4hep::Vector3f pairPos = 0.5*(h1AtDcaTo2 + h2AtDcaTo1);
       
       edm4hep::Vector3f h1MomAtDca = h1.momentumAt(ss.first, m_cfg.b_field);
       edm4hep::Vector3f h2MomAtDca = h2.momentumAt(ss.second, m_cfg.b_field);
       edm4hep::Vector3f pairMom = h1MomAtDca + h2MomAtDca;
       
       double e1 = std::hypot(edm4hep::utils::magnitude(h1MomAtDca), particleSvc.particle(211).mass);
       double e2 = std::hypot(edm4hep::utils::magnitude(h2MomAtDca), particleSvc.particle(211).mass);
       double pairE = e1+e2;
       double pairP = edm4hep::utils::magnitude(pairMom);
       
       double m_inv2 = pairE*pairE - pairP*pairP;
       double m_inv = (m_inv2>0) sqrt(m_inv2) : 0.;
       double angle = edm4hep::utils::angleBetween(pairMom, pairPos - pVtxPos);
       if(cos(angle) < m_cfg.minCostheta ) continue;

       double beta = edm4hep::utils::magnitude(pairMom)/pairE;
       double time = edm4hep::utils::magnitude(pairPos - pVtxPos)/(beta*dd4hep::c_light);
       auto v0 = out_secondary_vertices->create();
       v0.setType(2); // 2 for secondary
       v0.setPosition({(float)(pairPos.x * edm4eic::unit::cm / edm4eic::unit::mm), 
                       (float)(pairPos.y * edm4eic::unit::cm / edm4eic::unit::mm),
                       (float)(pairPos.z * edm4eic::unit::cm / edm4eic::unit::mm), 
                       (float)time});
       v0.addToAssociatedParticles(p1);
       v0.addToAssociatedParticles(p2);                
       
       info("One secondary vertex found at (x,y,z) = ({}, {}, {}) mm.",
                 pairPos.x * edm4eic::unit::cm / edm4eic::unit::mm,
                 pairPos.y * edm4eic::unit::cm / edm4eic::unit::mm,
                 pairPos.x * edm4eic::unit::cm / edm4eic::unit::mm);
                 
    } // end i2
  } // end i1
  
} // end process

} // namespace eicrecon
