// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Daniel Brandenburg

#include <Math/GenVector/LorentzVector.h>
#include <Math/GenVector/PxPyPzM4D.h>
#include <Math/Vector4Dfwd.h>
#include <edm4eic/VertexCollection.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <iterator>
#include <map>
#include <utility>

#include "algorithms/reco/PrimaryVertices.h"
#include "algorithms/reco/PrimaryVerticesConfig.h"

namespace eicrecon {

  /**
   * @brief Initialize the PrimaryVertices Algorithm
   *
   * @param logger
   */
  void PrimaryVertices::init(std::shared_ptr<spdlog::logger>& logger) {
    m_log = logger;
  }

  /**
   * @brief Produce a list of primary vertex candidates
   *
   * @param rcvtx  - input collection of all vertex candidates
   * @return std::unique_ptr<edm4eic::VertexCollection>
   */
  std::unique_ptr<edm4eic::VertexCollection> PrimaryVertices::execute(
                const edm4eic::VertexCollection *rcvtx
        ){

    // this map will store intermediate results
    // so that we can sort them before filling output
    // collection
    std::map<int, edm4eic::Vertex> primaryVertexMap;

    // our output collection of primary vertex
    // ordered by N_trk = associatedParticle array size
    auto out_primary_vertices =  std::make_unique<edm4eic::VertexCollection>();
    out_primary_vertices->setSubsetCollection();

    m_log->trace( "We have {} candidate vertices",
        rcvtx->size()
      );

    for ( const auto& vtx: *rcvtx ) {
    
      const auto &v = vtx.getPosition();
      
      // some basic vertex selection
      if ( sqrt( v.x*v.x + v.y*v.y ) / edm4eic::unit::mm > m_cfg.maxVr ||
           fabs( v.z ) / edm4eic::unit::mm > m_cfg.maxVz )
           continue;
           
      if ( vtx.getChi2() > m_cfg.maxChi2 ) continue;
      
      //          
      int N_trk = vtx.getAssociatedParticles().size();
      m_log->trace( "\t N_trk = {}", N_trk );
      primaryVertexMap[ N_trk ] = vtx;
    } // vertex loop

    // map sorts in descending order by default
    // sort by descending
    bool first = true;
    // for (auto kv : primaryVertexMap) {
    for (auto kv = primaryVertexMap.rbegin(); kv != primaryVertexMap.rend(); ++kv) {

      int N_trk = kv->first;
      // Do not save primary candidates that
      // are not within range
      if ( N_trk > m_cfg.maxNtrk
        || N_trk < m_cfg.minNtrk ){
        continue;
      }

      // For logging and development
      // report the highest N_trk candidate chosen
      if ( first ){
        m_log->trace( "Max N_trk Candidate:" );
        m_log->trace( "\t N_trk = {}", N_trk );
        m_log->trace( "\t Primary vertex has xyz=( {}, {}, {} )", kv->second.getPosition().x, kv->second.getPosition().y, kv->second.getPosition().z );
        first = false;
      }
      out_primary_vertices->push_back( kv->second );
    } // reverse loop on primaryVertexMap


    // Return primary vertex ranked
    // in order from largest N_trk to smallest
    return out_primary_vertices;
  }

} // namespace eicrecon
