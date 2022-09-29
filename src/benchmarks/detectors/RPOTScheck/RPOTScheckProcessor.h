
//
// Template for this file generated with eicmkplugin.py
//

#include <JANA/JEventProcessorSequentialRoot.h>
#include <TH2.h>
#include <TFile.h>

#include <edm4hep/SimTrackerHit.h>
#include <edm4eic/RawTrackerHit.h>
#include <edm4eic/TrackerHit.h>
#include <edm4eic/ReconstructedParticle.h>


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
