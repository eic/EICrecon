
//
// Template for this file generated with eicmkplugin.py
//

#include "EcalBarrelScFiCheckProcessor.h"
#include "services/rootfile/RootFile_service.h"

#include <Evaluator/DD4hepUnits.h>

// The following just makes this a JANA plugin
extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new EcalBarrelScFiCheckProcessor);
    }
}

//-------------------------------------------
// InitWithGlobalRootLock
//-------------------------------------------
void EcalBarrelScFiCheckProcessor::InitWithGlobalRootLock(){

    auto rootfile_svc = GetApplication()->GetService<RootFile_service>();
    auto rootfile = rootfile_svc->GetHistFile();
    rootfile->mkdir("EcalBarrelScFi")->cd();

    hist1D["EcalBarrelScFiHits_hits_per_event"]  =  new TH1I("EcalBarrelScFiHits_hits_per_event",  "EcalBarrelScFi Simulated hit Nhits/event;Nhits",  300, 0.0, 3000);
    hist2D["EcalBarrelScFiHits_occupancy"]  =  new TH2I("EcalBarrelScFiHits_occupancy",  "EcalBarrelScFi Simulated hit occupancy;column;row",  70, -700.0, 700.0,  70, -700.0, 700.0);
    hist1D["EcalBarrelScFiHits_hit_energy"] =  new TH1D("EcalBarrelScFiHits_hit_energy",  "EcalBarrelScFi Simulated hit energy;GeV",  1000, 0.0, 2.0);

    hist1D["EcalBarrelScFiRawhits_hits_per_event"]  =  new TH1I("EcalBarrelScFiRawhits_hits_per_event",  "EcalBarrelScFi Simulated digitized hit Nhits/event;Nhits",  300, 0.0, 3000);
    hist1D["EcalBarrelScFiRawhits_amplitude"] =  new TH1D("EcalBarrelScFiRawhits_amplitude",  "EcalBarrelScFi Simulated digitized hit amplitude;amplitude",  1000, 0.0, 8200.0);
    hist1D["EcalBarrelScFiRawhits_timestamp"] =  new TH1I("EcalBarrelScFiRawhits_timestamp",  "EcalBarrelScFi Simulated digitized hit timestamp;timestamp",  1024, 0.0, 8191.0);

    hist1D["EcalBarrelScFiRechits_hits_per_event"]  =  new TH1I("EcalBarrelScFiRechits_hits_per_event",  "EcalBarrelScFi Reconstructed hit Nhits/event;Nhits",  300, 0.0, 3000);
    hist1D["EcalBarrelScFiRecHits_hit_energy"] =  new TH1D("EcalBarrelScFiRecHits_hit_energy",  "EcalBarrelScFi Reconstructed hit energy;MeV",  1000, 0.0, 100.0);
    hist2D["EcalBarrelScFiRecHits_xy"]  =  new TH2D("EcalBarrelScFiRecHits_xy",  "EcalBarrelScFi Reconstructed hit Y vs. X (energy weighted);x;y",  128, -1100.0, 1100.0,  128, -1100.0, 1100.0);
    hist1D["EcalBarrelScFiRecHits_z"]  =  new TH1D("EcalBarrelScFiRecHits_z",  "EcalBarrelScFi Reconstructed hit Z;z",  400, -3000.0, 1600.0);
    hist1D["EcalBarrelScFiRecHits_time"]  =  new TH1D("EcalBarrelScFiRecHits_time",  "EcalBarrelScFi Reconstructed hit time;time",  1000, -10.0, 2000.0);

    hist1D["EcalBarrelScFiProtoClusters_clusters_per_event"]  =  new TH1I("EcalBarrelScFiProtoClusters_clusters_per_event",  "EcalBarrelScFi Protoclusters Nclusters/event;Nclusters",  61, -0.5, 60.5);
    hist1D["EcalBarrelScFiProtoClusters_hits_per_cluster"] = new TH1I("EcalBarrelScFiProtoClusters_hits_per_cluster",  "EcalBarrelScFi Protoclusters Nhits/cluster;Nhits",  101, -0.5, 100.5);

    // Set some draw options
    hist2D["EcalBarrelScFiHits_occupancy"]->SetOption("colz");
    hist2D["EcalBarrelScFiRecHits_xy"]->SetOption("colz");
}

//-------------------------------------------
// ProcessSequential
//-------------------------------------------
void EcalBarrelScFiCheckProcessor::ProcessSequential(const std::shared_ptr<const JEvent>& event) {

    // Fill histograms here

    // EcalBarrelScFiHits
    hist1D["EcalBarrelScFiHits_hits_per_event"]->Fill(EcalBarrelScFiHits().size());
    for( auto hit : EcalBarrelScFiHits()  ){
        auto row = floor(hit->getPosition().y);
        auto col = floor(hit->getPosition().x);
        hist2D["EcalBarrelScFiHits_occupancy"]->Fill(row, col);

        hist1D["EcalBarrelScFiHits_hit_energy"]->Fill(hit->getEnergy());
    }

    // EcalBarrelScFiRawhits
    hist1D["EcalBarrelScFiRawhits_hits_per_event"]->Fill(EcalBarrelScFiRawhits().size());
    for( auto hit : EcalBarrelScFiRawhits()  ){
        hist1D["EcalBarrelScFiRawhits_amplitude"]->Fill( hit->getAmplitude() );
        hist1D["EcalBarrelScFiRawhits_timestamp"]->Fill( hit->getTimeStamp() );
    }

    // EcalBarrelScFiRechits
    hist1D["EcalBarrelScFiRechits_hits_per_event"]->Fill(EcalBarrelScFiRecHits().size());
    for( auto hit : EcalBarrelScFiRecHits()  ){
        auto &pos = hit->getPosition();
        hist1D["EcalBarrelScFiRecHits_hit_energy"]->Fill(hit->getEnergy() / dd4hep::MeV);
        hist2D["EcalBarrelScFiRecHits_xy"]->Fill( pos.x, pos.y, hit->getEnergy() );
        hist1D["EcalBarrelScFiRecHits_z"]->Fill(pos.z);
        hist1D["EcalBarrelScFiRecHits_time"]->Fill( hit->getTime() );
    }


    // EcalBarrelScFiProtoClusters
    hist1D["EcalBarrelScFiProtoClusters_clusters_per_event"]->Fill(EcalBarrelScFiProtoClusters().size());
    for (auto proto : EcalBarrelScFiProtoClusters() ){
        hist1D["EcalBarrelScFiProtoClusters_hits_per_cluster"]->Fill( proto->getHits().size() );
    }
}

//-------------------------------------------
// FinishWithGlobalRootLock
//-------------------------------------------
void EcalBarrelScFiCheckProcessor::FinishWithGlobalRootLock() {

    // Do any final calculations here.

}
