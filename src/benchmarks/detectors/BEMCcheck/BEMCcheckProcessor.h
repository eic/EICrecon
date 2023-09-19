
//
// Template for this file generated with eicmkplugin.py
//

#include <JANA/JEvent.h>
#include <JANA/JEventProcessorSequentialRoot.h>
#include <JANA/Utils/JTypeInfo.h>
#include <TH1.h>
#include <TH2.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <algorithm>
#include <map>
#include <memory>
#include <string>


// Include appropirate class headers. e.g.
// #include <edm4hep/SimCalorimeterHit.h>
// #include "detectors/BEMC/BEMCRawCalorimeterHit.h"


class BEMCcheckProcessor: public JEventProcessorSequentialRoot {
private:

    // Data objects we will need from JANA e.g.
    PrefetchT<edm4hep::SimCalorimeterHit> EcalBarrelhits                 = {this, "EcalBarrelHits"};
    PrefetchT<edm4hep::RawCalorimeterHit> EcalBarrelRawhits              = {this, "EcalBarrelRawHits"};
    PrefetchT<edm4eic::CalorimeterHit>    EcalBarrelRecHits              = {this, "EcalBarrelRecHits"};
    PrefetchT<edm4eic::ProtoCluster>      EcalBarrelIslandProtoClusters  = {this, "EcalBarrelIslandProtoClusters"};

    // Declare histogram and tree pointers
    std::map<std::string, TH1*> hist1D;
    std::map<std::string, TH2*> hist2D;

public:
    BEMCcheckProcessor() { SetTypeName(NAME_OF_THIS); }

    void InitWithGlobalRootLock() override;
    void ProcessSequential(const std::shared_ptr<const JEvent>& event) override;
    void FinishWithGlobalRootLock() override;
};
