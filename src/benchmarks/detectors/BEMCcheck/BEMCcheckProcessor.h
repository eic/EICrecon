
//
// Template for this file generated with eicmkplugin.py
//

#include <JANA/JEventProcessorSequentialRoot.h>
#include <TH2.h>
#include <TFile.h>

#include <edm4hep/SimCalorimeterHit.h>
#include <edm4hep/RawCalorimeterHit.h>
#include <edm4eic/ProtoCluster.h>


// Include appropirate class headers. e.g.
// #include <edm4hep/SimCalorimeterHit.h>
// #include <detectors/BEMC/BEMCRawCalorimeterHit.h>


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
