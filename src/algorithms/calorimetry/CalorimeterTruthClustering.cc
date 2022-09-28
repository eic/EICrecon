

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

    std::vector<edm4eic::MutableProtoCluster*> proto;

    // Map mc track ID to protoCluster index
    std::map<int32_t, int32_t> protoIndex;

    // Loop over all calorimeter hits and sort per mcparticle
    for (const auto& hit : hits) {
        // The original algorithm used the following to get the mcHit:
        //
        //    const auto& mcHit     = mc[hit->getObjectID().index];
        //
        // This assumes there is a one-to-one relation between the truth hit
        // (hits) and the reconstructed hit (mc). At least insofar as the
        // index in "hits" is being used to index the "mc" container.
        //
        // If the objects in "hits" have not been added to a collection,
        // then they will have getObjectID().index = "untracked" = -1
        //
        // The way we handle this is here is to check if getObjectID().index
        // is within the size limits of mc which includes >=0. If so, then
        // assume the old code is valid. If not, then we need to search
        // for the right hit.
        // FIXME: This is clearly not the right way to do this! Podio needs
        // FIXME: to be fixed so proper object tracking can be done without
        // FIXME: requiring Collection classes be used to manage all objects.
        const edm4hep::SimCalorimeterHit* mcHit = nullptr;
        if( hit->getObjectID().index>=0 && hit->getObjectID().index<mc.size() ){
            mcHit = mc[hit->getObjectID().index];
        }else{
            for( auto tmpmc : mc ){
                if( tmpmc->getCellID() == hit->getCellID() ){
                    mcHit = tmpmc;
                    break;
                }
            }
        }
        if( ! mcHit ) continue; // ignore hit if we couldn't match it to truth hit

        const auto &trackID = mcHit->getContributions(0).getParticle().id();
        // Create a new protocluster if we don't have one for this trackID
        if (protoIndex.count(trackID) == 0) {
            proto.push_back( new edm4eic::MutableProtoCluster() );
            protoIndex[trackID] = proto.size() - 1;
        }
        // Add hit to the appropriate protocluster
        proto[protoIndex[trackID]]->addToHits(*hit);
        proto[protoIndex[trackID]]->addToWeights(1);
    }

    // iterate over proto and push to m_outputProtoClusters
    for (auto& p : proto) {
      edm4eic::ProtoCluster* to_add=new edm4eic::ProtoCluster(*p);
      m_outputProtoClusters.push_back(to_add);
    }

    return;
}
