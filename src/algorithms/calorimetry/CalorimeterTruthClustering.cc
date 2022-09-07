

#include "CalorimeterTruthClustering.h"

#include <JANA/JEvent.h>
#include <Evaluator/DD4hepUnits.h>
#include <fmt/format.h>
#include <edm4hep/MCParticle.h>

using namespace dd4hep;

//this algorithm converted from https://eicweb.phy.anl.gov/EIC/juggler/-/blob/master/JugReco/src/components/CalorimeterHitReco.cpp


//------------------------
// AlgorithmInit
//------------------------
void CalorimeterTruthClustering::AlgorithmInit() {

    return;
}

//------------------------
// AlgorithmChangeRun
//------------------------
void CalorimeterTruthClustering::AlgorithmChangeRun() {
}

//------------------------
// AlgorithmProcess
//------------------------
void CalorimeterTruthClustering::AlgorithmProcess() {

    // input collections
    const auto& hits = m_inputHits;
    const auto& mc   = m_mcHits;
    // Create output collections
    auto& proto = m_outputProtoClusters;

    // Map mc track ID to protoCluster index
    std::map<int32_t, int32_t> protoIndex;

    // Loop over al calorimeter hits and sort per mcparticle
    for (const auto& hit : hits) {
      const auto& mcHit     = mc[hit->getObjectID().index];
      const auto& trackID   = mcHit->getContributions(0).getParticle().id();
      // Create a new protocluster if we don't have one for this trackID
      if (protoIndex.count(trackID) == 0) {
        auto pcl = new eicd::ProtoCluster();
        protoIndex[trackID] = proto.size() - 1;
      }
      // Add hit to the appropriate protocluster
      proto[protoIndex[trackID]]->addToHits(*hit);
      proto[protoIndex[trackID]]->addToWeights(1);
    }

    return;
}
