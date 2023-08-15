
//
// Template for this file generated with eicmkplugin.py
//

#include <JANA/JEventProcessorSequentialRoot.h>
#include <TH2.h>
#include <TFile.h>

#include <edm4hep/SimCalorimeterHit.h>
#include <edm4hep/RawCalorimeterHit.h>
#include <edm4eic/ProtoCluster.h>

class EcalBarrelScFiCheckProcessor: public JEventProcessorSequentialRoot {
private:

    // Data objects we will need from JANA e.g.
    PrefetchT<edm4hep::SimCalorimeterHit> EcalBarrelScFiHits           = {this, "EcalBarrelScFiHits"};
    PrefetchT<edm4hep::RawCalorimeterHit> EcalBarrelScFiRawhits        = {this, "EcalBarrelScFiRawHits"};
    PrefetchT<edm4eic::CalorimeterHit>    EcalBarrelScFiRecHits        = {this, "EcalBarrelScFiRecHits"};
    PrefetchT<edm4eic::ProtoCluster>      EcalBarrelScFiProtoClusters  = {this, "EcalBarrelScFiProtoClusters"};

    // Declare histogram and tree pointers
    std::map<std::string, TH1*> hist1D;
    std::map<std::string, TH2*> hist2D;

public:
    EcalBarrelScFiCheckProcessor() { SetTypeName(NAME_OF_THIS); }

    void InitWithGlobalRootLock() override;
    void ProcessSequential(const std::shared_ptr<const JEvent>& event) override;
    void FinishWithGlobalRootLock() override;
};
