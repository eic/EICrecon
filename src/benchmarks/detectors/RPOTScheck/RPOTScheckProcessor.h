
//
// Template for this file generated with eicmkplugin.py
//

#include <JANA/JEvent.h>
#include <JANA/JEventProcessorSequentialRoot.h>
#include <JANA/Utils/JTypeInfo.h>
#include <TH1.h>
#include <TH2.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <algorithm>
#include <map>
#include <memory>
#include <string>


class RPOTScheckProcessor: public JEventProcessorSequentialRoot {
private:

    // Data objects we will need from JANA
    PrefetchT<edm4hep::SimTrackerHit>         ForwardRomanPotHits     = {this, "ForwardRomanPotHits"};
    PrefetchT<edm4eic::RawTrackerHit>         ForwardRomanPotRawHits    = {this, "ForwardRomanPotRawHits"};
    PrefetchT<edm4eic::TrackerHit>            ForwardRomanPotRecHits    = {this, "ForwardRomanPotRecHits"};
    PrefetchT<edm4eic::ReconstructedParticle> ForwardRomanPotParticles  = {this, "ForwardRomanPotParticles"};

    // Containers for histograms
    std::map<std::string, TH1*> hist1D;
    std::map<std::string, TH2*> hist2D;

public:
    RPOTScheckProcessor() { SetTypeName(NAME_OF_THIS); }

    void InitWithGlobalRootLock() override;
    void ProcessSequential(const std::shared_ptr<const JEvent>& event) override;
    void FinishWithGlobalRootLock() override;
};
