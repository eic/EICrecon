
//
// Template for this file generated with eicmkplugin.py
//

#include "BEMCcheckProcessor.h"
#include "services/rootfile/RootFile_service.h"

#include <Evaluator/DD4hepUnits.h>

// The following just makes this a JANA plugin
extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new BEMCcheckProcessor);
    }
}

//-------------------------------------------
// InitWithGlobalRootLock
//-------------------------------------------
void BEMCcheckProcessor::InitWithGlobalRootLock(){

    auto rootfile_svc = GetApplication()->GetService<RootFile_service>();
    auto rootfile = rootfile_svc->GetHistFile();
    rootfile->mkdir("BEMC")->cd();

    hist1D["EcalBarrelhits_hits_per_event"]  =  new TH1I("EcalBarrelhits_hits_per_event",  "BEMC Simulated hit Nhits/event;Nhits",  300, 0.0, 3000);
//    hist2D["EcalBarrelhits_occupancy"]  =  new TH2I("EcalBarrelhits_occupancy",  "BEMC Simulated hit occupancy;column;row",  64, -32.0, 32.0,  64, -32.0, 32.0);
    hist1D["EcalBarrelhits_hit_energy"] =  new TH1D("EcalBarrelhits_hit_energy",  "BEMC Simulated hit energy;GeV",  1000, 0.0, 2.0);

    hist1D["EcalBarrelRawhits_hits_per_event"]  =  new TH1I("EcalBarrelRawhits_hits_per_event",  "BEMC Simulated digitized hit Nhits/event;Nhits",  300, 0.0, 3000);
    hist1D["EcalBarrelRawhits_amplitude"] =  new TH1D("EcalBarrelRawhits_amplitude",  "BEMC Simulated digitized hit amplitude;amplitude",  1000, 0.0, 8200.0);
    hist1D["EcalBarrelRawhits_timestamp"] =  new TH1I("EcalBarrelRawhits_timestamp",  "BEMC Simulated digitized hit timestamp;timestamp",  1024, 0.0, 8191.0);

    hist1D["EcalBarrelRechits_hits_per_event"]  =  new TH1I("EcalBarrelRechits_hits_per_event",  "BEMC Reconstructed hit Nhits/event;Nhits",  300, 0.0, 3000);
    hist1D["EcalBarrelRecHits_hit_energy"] =  new TH1D("EcalBarrelRecHits_hit_energy",  "BEMC Reconstructed hit energy;MeV",  1000, 0.0, 100.0);
    hist2D["EcalBarrelRecHits_xy"]  =  new TH2D("EcalBarrelRecHits_xy",  "BEMC Reconstructed hit Y vs. X (energy weighted);x;y",  128, -1100.0, 1100.0,  128, -1100.0, 1100.0);
    hist1D["EcalBarrelRecHits_z"]  =  new TH1D("EcalBarrelRecHits_z",  "BEMC Reconstructed hit Z;z",  400, -3000.0, 1600.0);
    hist1D["EcalBarrelRecHits_time"]  =  new TH1D("EcalBarrelRecHits_time",  "BEMC Reconstructed hit time;time",  1000, -10.0, 2000.0);

    hist1D["EcalBarrelIslandProtoClusters_clusters_per_event"]  =  new TH1I("EcalBarrelIslandProtoClusters_clusters_per_event",  "BEMC Protoclusters Nclusters/event;Nclusters",  61, -0.5, 60.5);
    hist1D["EcalBarrelIslandProtoClusters_hits_per_cluster"] = new TH1I("EcalBarrelIslandProtoClusters_hits_per_cluster",  "BEMC Protoclusters Nhits/cluster;Nhits",  101, -0.5, 100.5);

    // Set some draw options
//    hist2D["EcalBarrelhits_occupancy"]->SetOption("colz");
    hist2D["EcalBarrelRecHits_xy"]->SetOption("colz");
}

//-------------------------------------------
// ProcessSequential
//-------------------------------------------
void BEMCcheckProcessor::ProcessSequential(const std::shared_ptr<const JEvent>& event) {

    // Fill histograms here

    // EcalBarrelhits
    hist1D["EcalBarrelhits_hits_per_event"]->Fill(EcalBarrelhits().size());
    for( auto hit : EcalBarrelhits()  ){
//        auto row = floor(hit->getPosition().y/20.5); // 20.5 is empirical
//        auto col = floor(hit->getPosition().x/20.5); // 20.5 is empirical
//        hist2D["EcalBarrelhits_occupancy"]->Fill(row, col);

        hist1D["EcalBarrelhits_hit_energy"]->Fill(hit->getEnergy());
    }

    // EcalBarrelRawhits
    hist1D["EcalBarrelRawhits_hits_per_event"]->Fill(EcalBarrelRawhits().size());
    for( auto hit : EcalBarrelRawhits()  ){
        hist1D["EcalBarrelRawhits_amplitude"]->Fill( hit->getAmplitude() );
        hist1D["EcalBarrelRawhits_timestamp"]->Fill( hit->getTimeStamp() );
    }

    // EcalBarrelRechits
    hist1D["EcalBarrelRechits_hits_per_event"]->Fill(EcalBarrelRecHits().size());
    for( auto hit : EcalBarrelRecHits()  ){
        auto &pos = hit->getPosition();
        hist1D["EcalBarrelRecHits_hit_energy"]->Fill(hit->getEnergy() / dd4hep::MeV);
        hist2D["EcalBarrelRecHits_xy"]->Fill( pos.x, pos.y, hit->getEnergy() );
        hist1D["EcalBarrelRecHits_z"]->Fill(pos.z);
        hist1D["EcalBarrelRecHits_time"]->Fill( hit->getTime() );
    }


    // EcalBarrelIslandProtoClusters
    hist1D["EcalBarrelIslandProtoClusters_clusters_per_event"]->Fill(EcalBarrelIslandProtoClusters().size());
    for (auto proto : EcalBarrelIslandProtoClusters() ){
        hist1D["EcalBarrelIslandProtoClusters_hits_per_cluster"]->Fill( proto->getHits().size() );
    }
}

//-------------------------------------------
// FinishWithGlobalRootLock
//-------------------------------------------
void BEMCcheckProcessor::FinishWithGlobalRootLock() {

    // Do any final calculations here.

}
