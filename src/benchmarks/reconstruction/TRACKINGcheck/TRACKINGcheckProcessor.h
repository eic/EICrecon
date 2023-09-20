
//
// Template for this file generated with eicmkplugin.py
//

#include <JANA/JEventProcessorSequentialRoot.h>
#include <TH2.h>
#include <TFile.h>

#include <edm4hep/MCParticle.h>
#include "algorithms/tracking/ActsExamples/EventData/Trajectories.hpp"


class TRACKINGcheckProcessor: public JEventProcessorSequentialRoot {
private:

    // Data objects we will need from JANA
    PrefetchT<edm4hep::MCParticle>  MCParticles   = {this, "MCParticles" };
    PrefetchT<ActsExamples::Trajectories>    Trajectories  = {this, "CentralCKFActsTrajectories"};

    // Containers for histograms
    std::map<std::string, TH1*> hist1D;
    std::map<std::string, TH2*> hist2D;

public:
    TRACKINGcheckProcessor() { SetTypeName(NAME_OF_THIS); }

    void InitWithGlobalRootLock() override;
    void ProcessSequential(const std::shared_ptr<const JEvent>& event) override;
    void FinishWithGlobalRootLock() override;
};
