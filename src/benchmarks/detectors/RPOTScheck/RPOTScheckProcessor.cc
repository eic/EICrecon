
//
// Template for this file generated with eicmkplugin.py
//

#include "RPOTScheckProcessor.h"
#include "services/rootfile/RootFile_service.h"

#include <Evaluator/DD4hepUnits.h>
#include <TVector3.h>

// The following just makes this a JANA plugin
extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new RPOTScheckProcessor);
    }
}

//-------------------------------------------
// InitWithGlobalRootLock
//-------------------------------------------
void RPOTScheckProcessor::InitWithGlobalRootLock(){

    auto rootfile_svc = GetApplication()->GetService<RootFile_service>();
    auto rootfile = rootfile_svc->GetHistFile();
    rootfile->mkdir("RPOTS")->cd();

    hist1D["ForwardRomanPotHits_hits_per_event"]  =  new TH1I("ForwardRomanPotHits_hits_per_event",  "RPOTS Simulated hit Nhits/event;Nhits",  201, -0.5, 200.5);
    hist1D["ForwardRomanPotHits_EDep"] =  new TH1D("ForwardRomanPotHits_EDep",  "RPOTS Simulated hit energy;MeV",  1000, 0.0, 5.0);
    hist1D["ForwardRomanPotHits_time"] =  new TH1D("ForwardRomanPotHits_time",  "RPOTS Simulated hit time;time (ns)",  150, 85.0, 100.0);
    hist1D["ForwardRomanPotHits_pathlength"] =  new TH1D("ForwardRomanPotHits_pathlength",  "RPOTS Simulated hit path length;path length",  200, 0.0, 5.0);
    hist2D["ForwardRomanPotHits_xy"]  =  new TH2D("ForwardRomanPotHits_xy",  "RPOTS Simulated hit hit Y vs. X;x;y",  100, -1070.0, -680.0,  100, -80., 80.0);
    hist1D["ForwardRomanPotHits_z"] =  new TH1D("ForwardRomanPotHits_z",  "RPOTS Simulated hit Z;z",  200, 25000.0, 29000.0);
    hist1D["ForwardRomanPotHits_p"] =  new TH1D("ForwardRomanPotHits_p",  "RPOTS Simulated hit momentum;p",  1000, 0.0, 10.0);
    hist1D["ForwardRomanPotHits_theta"] =  new TH1D("ForwardRomanPotHits_theta",  "RPOTS Simulated hit #theta;#theta (mrad)",  400, 0.0, 2000.0);
    hist1D["ForwardRomanPotHits_phi"] =  new TH1D("ForwardRomanPotHits_phi",  "RPOTS Simulated hit #phi;#phi (rad)",  200, -3.1416, 3.1416);

    hist1D["ForwardRomanPotRawHits_hits_per_event"]  =  new TH1I("ForwardRomanPotRawHits_hits_per_event",  "RPOTS Simulated digitized hit Nhits/event;Nhits",  201, -0.5, 200.5);
    hist1D["ForwardRomanPotRawHits_charge"] =  new TH1D("ForwardRomanPotRawHits_charge",  "RPOTS Simulated digitized hit charge;charge",  400, 0.0, 3.5E5);
    hist1D["ForwardRomanPotRawHits_timestamp"] =  new TH1I("ForwardRomanPotRawHits_timestamp",  "RPOTS Simulated digitized hit timestamp;timestamp",  1024, 0.0, 8191.0);

    hist1D["ForwardRomanPotRecHits_hits_per_event"]  =  new TH1I("ForwardRomanPotRecHits_hits_per_event",  "RPOTS Reconstructed hit Nhits/event;Nhits",  201, -0.5, 200.5);
    hist1D["ForwardRomanPotRecHits_time"]  =  new TH1D("ForwardRomanPotRecHits_time",  "RPOTS Reconstructed hit time;time",  200, -300.0, 300.0);
    hist1D["ForwardRomanPotRecHits_Edep"] =  new TH1D("ForwardRomanPotRecHits_EDep",  "RPOTS Reconstructed hit energy;MeV",  200, 0.0, 8.0E4);
    hist2D["ForwardRomanPotRecHits_xy"]  =  new TH2D("ForwardRomanPotRecHits_xy",  "RPOTS Reconstructed hit hit Y vs. X;x;y",  100, -1070.0, -680.0,  100, -80., 80.0);
    hist1D["ForwardRomanPotRecHits_z"] =  new TH1D("ForwardRomanPotRecHits_z",  "RPOTS Reconstructed hit Z;z",  200, 25000.0, 29000.0);

    hist1D["FarForwardParticles_particles_per_event"]  =  new TH1I("FarForwardParticles_particles_per_event",  "RPOTS Reconstructed particles/event;Nparticles",  201, -0.5, 200.5);

    // Set some draw options
    hist2D["ForwardRomanPotHits_xy"]->SetOption("colz");
    hist2D["ForwardRomanPotRecHits_xy"]->SetOption("colz");
}

//-------------------------------------------
// ProcessSequential
//-------------------------------------------
void RPOTScheckProcessor::ProcessSequential(const std::shared_ptr<const JEvent>& event) {

    // Fill histograms here

    // ForwardRomanPotHits
    hist1D["ForwardRomanPotHits_hits_per_event"]->Fill(ForwardRomanPotHits().size());

    for( auto hit : ForwardRomanPotHits() ){
        hist1D["ForwardRomanPotHits_EDep"]->Fill( hit->getEDep() / dd4hep::MeV);
        hist1D["ForwardRomanPotHits_time"]->Fill( hit->getTime());
        hist1D["ForwardRomanPotHits_pathlength"]->Fill( hit->getPathLength());

        hist2D["ForwardRomanPotHits_xy"]->Fill( hit->getPosition().x, hit->getPosition().y);
        hist1D["ForwardRomanPotHits_z"]->Fill( hit->getPosition().z);

        TVector3 mom( hit->getMomentum().x, hit->getMomentum().y, hit->getMomentum().z );
        hist1D["ForwardRomanPotHits_p"]->Fill( mom.Mag() );
        hist1D["ForwardRomanPotHits_theta"]->Fill( mom.Theta()*1000.0 );
        hist1D["ForwardRomanPotHits_phi"]->Fill( mom.Phi() );
    }

    // ForwardRomanPotRawHits
    hist1D["ForwardRomanPotRawHits_hits_per_event"]->Fill(ForwardRomanPotRawHits().size());
    for( auto hit : ForwardRomanPotRawHits()  ){
        hist1D["ForwardRomanPotRawHits_charge"]->Fill( hit->getCharge() );
        hist1D["ForwardRomanPotRawHits_timestamp"]->Fill( hit->getTimeStamp() );
    }

    // ForwardRomanPotRecHits
    hist1D["ForwardRomanPotRecHits_hits_per_event"]->Fill(ForwardRomanPotRecHits().size());
    for( auto hit : ForwardRomanPotRecHits() ){
        hist1D["ForwardRomanPotRecHits_Edep"]->Fill( hit->getEdep() / dd4hep::MeV);
        hist1D["ForwardRomanPotRecHits_time"]->Fill( hit->getTime());

        hist2D["ForwardRomanPotRecHits_xy"]->Fill( hit->getPosition().x, hit->getPosition().y);
        hist1D["ForwardRomanPotRecHits_z"]->Fill( hit->getPosition().z);
    }

    hist1D["FarForwardParticles_particles_per_event"]->Fill(ForwardRomanPotParticles().size());
}

//-------------------------------------------
// FinishWithGlobalRootLock
//-------------------------------------------
void RPOTScheckProcessor::FinishWithGlobalRootLock() {

    // Do any final calculations here.

}
