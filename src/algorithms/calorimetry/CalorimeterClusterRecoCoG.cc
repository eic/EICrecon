
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Chao, Chao Peng, Whitney Armstrong

/*
 *  Reconstruct the cluster with Center of Gravity method
 *  Logarithmic weighting is used for mimicing energy deposit in transverse direction
 *
 *  Author: Chao Peng (ANL), 09/27/2020
 */
#include "CalorimeterClusterRecoCoG.h"

#include <JANA/JEvent.h>
#include <Evaluator/DD4hepUnits.h>
#include <fmt/format.h>
#include <map>

#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/map.hpp>
#include <edm4hep/MCParticle.h>


using namespace dd4hep;

//------------------------
// AlgorithmInit
//------------------------

void CalorimeterClusterRecoCoG::AlgorithmInit(std::shared_ptr<spdlog::logger>& logger) {

    m_log=logger;
    // update depth correction if a name is provided
    if (m_moduleDimZName != "") {
      m_depthCorrection = m_geoSvc->detector()->constantAsDouble(m_moduleDimZName);
    }

    // select weighting method
    std::string ew = m_energyWeight;
    // make it case-insensitive
    std::transform(ew.begin(), ew.end(), ew.begin(), [](char s) { return std::tolower(s); });
    auto it = weightMethods.find(ew);
    if (it == weightMethods.end()) {
      //LOG_ERROR(default_cerr_logger) << fmt::format("Cannot find energy weighting method {}, choose one from [{}]", m_energyWeight, boost::algorithm::join(weightMethods | boost::adaptors::map_keys, ", ")) << LOG_END;
      m_log->error("Cannot find energy weighting method {}, choose one from [{}]", m_energyWeight, boost::algorithm::join(weightMethods | boost::adaptors::map_keys, ", "));
      return;
    }
    weightFunc = it->second;
    // info() << "z_length " << depth << endmsg;

    return;
}

//------------------------
// AlgorithmChangeRun
//------------------------
void CalorimeterClusterRecoCoG::AlgorithmChangeRun() {
}

//------------------------
// AlgorithmProcess
//------------------------
void CalorimeterClusterRecoCoG::AlgorithmProcess() {

    // input collections
    const auto& proto  = m_inputProto;
    auto& clusters     = m_outputClusters;

    // Optional input MC data
    std::vector<const edm4hep::SimCalorimeterHit*> mchits = m_inputSimhits;

    // Optional output associations
    //associations removed in favor of referencing underlying vector m_outputAssociations
    //std::vector<edm4eic::MCRecoClusterParticleAssociation*> associations = m_outputAssociations;


    for (const auto& pcl : proto) {
      auto cl = reconstruct(pcl);

      if (m_log->level() <= spdlog::level::debug) {
        //LOG_INFO(default_cout_logger) << cl.getNhits() << " hits: " << cl.getEnergy() / dd4hep::GeV << " GeV, (" << cl.getPosition().x / dd4hep::mm << ", " << cl.getPosition().y / dd4hep::mm << ", " << cl.getPosition().z / dd4hep::mm << ")" << LOG_END;
        m_log->debug("{} hits: {} GeV, ({}, {}, {})", cl->getNhits(), cl->getEnergy() / dd4hep::GeV, cl->getPosition().x / dd4hep::mm, cl->getPosition().y / dd4hep::mm, cl->getPosition().z / dd4hep::mm);
      }
      clusters.push_back(cl);

      // If mcHits are available, associate cluster with MCParticle
      // 1. find proto-cluster hit with largest energy deposition
      // 2. find first mchit with same CellID
      // 3. assign mchit's MCParticle as cluster truth
//      if (!mchits.empty() && !m_outputAssociations.empty()) {  // ? having m_outputAssociations be not empty doesn't make sense ?
      if (!mchits.empty() ) {

        // 1. find pclhit with largest energy deposition
        auto pclhits = pcl->getHits();
        auto pclhit = std::max_element(
          pclhits.begin(),
          pclhits.end(),
          [](const auto& pclhit1, const auto& pclhit2) {
            return pclhit1.getEnergy() < pclhit2.getEnergy();
          }
        );

        // FIXME: The code below fails for HcalEndcapPClusters. This does not happen for
        // FIXME: all calorimeters. A brief scan of the code suggests this could be caused
        // FIXME: by the CalorimeterHitDigi algorithm modifying the cellID for the raw hits.
        // FIXME: Thus, the cellID values passed on through to here no longer match those
        // FIXME: in the low-level truth hits. It likely works for other detectors because
        // FIXME: their u_fields and u_refs members are left empty which effectively results
        // FIXME: in the cellID being unchanged.

        // 2. find mchit with same CellID
        // find_if not working, https://github.com/AIDASoft/podio/pull/273
        //auto mchit = std::find_if(
        //  mchits.begin(),
        //  mchits.end(),
        //  [&pclhit](const auto& mchit1) {
        //    return mchit1.getCellID() == pclhit->getCellID();
        //  }
        //);
        auto mchit = mchits.begin();
        for ( ; mchit != mchits.end(); ++mchit) {
          // break loop when CellID match found
          if ( (*mchit)->getCellID() == pclhit->getCellID()) {
            break;
          }
        }
        if (!(mchit != mchits.end())) {
          // break if no matching hit found for this CellID
          //LOG_WARN(default_cout_logger) << "Proto-cluster has highest energy in CellID " << pclhit->getCellID() << ", but no mc hit with that CellID was found." << LOG_END;
          m_log->warn("Proto-cluster has highest energy in CellID {}, but no mc hit with that CellID was found.", pclhit->getCellID());
          //LOG_INFO(default_cout_logger) << "Proto-cluster hits: " << LOG_END;
          m_log->debug("Proto-cluster hits: ");
          for (const auto& pclhit1: pclhits) {
            //LOG_INFO(default_cout_logger) << pclhit1.getCellID() << ": " << pclhit1.getEnergy() << LOG_END;
            m_log->debug("{}: {}", pclhit1.getCellID(), pclhit1.getEnergy());
          }
          //LOG_INFO(default_cout_logger) << "MC hits: " << LOG_END;
          m_log->debug("MC hits: ");
          for (const auto& mchit1: mchits) {
            //LOG_INFO(default_cout_logger) << mchit1->getCellID() << ": " << mchit1->getEnergy() << LOG_END;
            m_log->debug("{}: {}", mchit1->getCellID(), mchit1->getEnergy());
          }
          break;
        }

        // 3. find mchit's MCParticle
        const auto& mcp = (*mchit)->getContributions(0).getParticle();

        // debug output
        if (m_log->level() <= spdlog::level::debug) {
          //LOG_INFO(default_cout_logger) << "cluster has largest energy in cellID: " << pclhit->getCellID() << LOG_END;
          m_log->debug("cluster has largest energy in cellID: {}", pclhit->getCellID());
          //LOG_INFO(default_cout_logger) << "pcl hit with highest energy " << pclhit->getEnergy() << " at index " << pclhit->getObjectID().index << LOG_END;
          m_log->debug("pcl hit with highest energy {} at index {}", pclhit->getEnergy(), pclhit->getObjectID().index);
          //LOG_INFO(default_cout_logger) << "corresponding mc hit energy " << (*mchit)->getEnergy() << " at index " << (*mchit)->getObjectID().index << LOG_END;
          m_log->debug("corresponding mc hit energy {} at index {}", (*mchit)->getEnergy(), (*mchit)->getObjectID().index);
          //LOG_INFO(default_cout_logger) << "from MCParticle index " << mcp.getObjectID().index << ", PDG " << mcp.getPDG() << ", " << edm4eic::magnitude(mcp.getMomentum()) << LOG_END;
          m_log->debug("from MCParticle index {}, PDG {}, {}", mcp.getObjectID().index, mcp.getPDG(), edm4eic::magnitude(mcp.getMomentum()));
        }

        // set association
        edm4eic::MutableMCRecoClusterParticleAssociation* clusterassoc = new edm4eic::MutableMCRecoClusterParticleAssociation();
//        clusterassoc->setRecID(cl->getObjectID().index); // if not using collection, this is always set to -1
        clusterassoc->setRecID((uint32_t)((uint64_t)cl&0xFFFFFFFF)); // mask lower 32 bits of cluster pointer as unique ID FIXME:
        clusterassoc->setSimID(mcp.getObjectID().index);
        clusterassoc->setWeight(1.0);
        clusterassoc->setRec(*cl);
        //clusterassoc.setSim(mcp);
        edm4eic::MCRecoClusterParticleAssociation* cassoc = new edm4eic::MCRecoClusterParticleAssociation(*clusterassoc);
        m_outputAssociations.push_back(cassoc);
      } else {
        if (m_log->level() <= spdlog::level::debug) {
          //LOG_INFO(default_cout_logger) << "No mcHitCollection was provided, so no truth association will be performed." << LOG_END;
          m_log->debug("No mcHitCollection was provided, so no truth association will be performed.");
        }
      }
    }

    return;
}
