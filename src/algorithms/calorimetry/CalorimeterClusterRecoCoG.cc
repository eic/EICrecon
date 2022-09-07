

#include "CalorimeterClusterRecoCoG.h"

#include <JANA/JEvent.h>
#include <Evaluator/DD4hepUnits.h>
#include <fmt/format.h>
#include <map>

#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/map.hpp>
#include <edm4hep/MCParticle.h>
#include <eicd/MutableMCRecoClusterParticleAssociation.h>

using namespace dd4hep;

//------------------------
// AlgorithmInit
//------------------------
void CalorimeterClusterRecoCoG::AlgorithmInit() {

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
      LOG_ERROR(default_cerr_logger) << fmt::format("Cannot find energy weighting method {}, choose one from [{}]", m_energyWeight,
                             boost::algorithm::join(weightMethods | boost::adaptors::map_keys, ", "))
              << LOG_END;
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
    //std::vector<eicd::MCRecoClusterParticleAssociation*> associations = m_outputAssociations;


    for (const auto& pcl : proto) {
      auto cl = reconstruct(pcl);

      if (false) {
        LOG_INFO(default_cout_logger) << cl.getNhits() << " hits: " << cl.getEnergy() / GeV << " GeV, (" << cl.getPosition().x / mm << ", "
                << cl.getPosition().y / mm << ", " << cl.getPosition().z / mm << ")" << LOG_END;
      }
      clusters.push_back(&cl);

      // If mcHits are available, associate cluster with MCParticle
      // 1. find proto-cluster hit with largest energy deposition
      // 2. find first mchit with same CellID
      // 3. assign mchit's MCParticle as cluster truth
      if (!mchits.empty() && !m_outputAssociations.empty()) {

        // 1. find pclhit with largest energy deposition
        auto pclhits = pcl->getHits();
        auto pclhit = std::max_element(
          pclhits.begin(),
          pclhits.end(),
          [](const auto& pclhit1, const auto& pclhit2) {
            return pclhit1.getEnergy() < pclhit2.getEnergy();
          }
        );

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
          LOG_WARN(default_cout_logger) << "Proto-cluster has highest energy in CellID " << pclhit->getCellID()
                    << ", but no mc hit with that CellID was found." << LOG_END;
          LOG_INFO(default_cout_logger) << "Proto-cluster hits: " << LOG_END;
          for (const auto& pclhit1: pclhits) {
            LOG_INFO(default_cout_logger) << pclhit1.getCellID() << ": " << pclhit1.getEnergy() << LOG_END;
          }
          LOG_INFO(default_cout_logger) << "MC hits: " << LOG_END;
          for (const auto& mchit1: mchits) {
            LOG_INFO(default_cout_logger) << mchit1->getCellID() << ": " << mchit1->getEnergy() << LOG_END;
          }
          break;
        }

        // 3. find mchit's MCParticle
        const auto& mcp = (*mchit)->getContributions(0).getParticle();

        // debug output
        if (false) {
          LOG_INFO(default_cout_logger) << "cluster has largest energy in cellID: " << pclhit->getCellID() << LOG_END;
          LOG_INFO(default_cout_logger) << "pcl hit with highest energy " << pclhit->getEnergy() << " at index " << pclhit->getObjectID().index << LOG_END;
          LOG_INFO(default_cout_logger) << "corresponding mc hit energy " << (*mchit)->getEnergy() << " at index " << (*mchit)->getObjectID().index << LOG_END;
          LOG_INFO(default_cout_logger) << "from MCParticle index " << mcp.getObjectID().index << ", PDG " << mcp.getPDG() << ", " << eicd::magnitude(mcp.getMomentum()) << LOG_END;
        }

        // set association
        eicd::MutableMCRecoClusterParticleAssociation* clusterassoc = new eicd::MutableMCRecoClusterParticleAssociation();
        clusterassoc->setRecID(cl.getObjectID().index);
        clusterassoc->setSimID(mcp.getObjectID().index);
        clusterassoc->setWeight(1.0);
        clusterassoc->setRec(cl);
        //clusterassoc.setSim(mcp);
        m_outputAssociations.push_back(clusterassoc);
      } else {
        if (false) {
          LOG_INFO(default_cout_logger) << "No mcHitCollection was provided, so no truth association will be performed." << LOG_END;
        }
      }
    }

    return;
}
