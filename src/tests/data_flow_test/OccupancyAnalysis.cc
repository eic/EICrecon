#include "OccupancyAnalysis.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <fmt/core.h>

#include <edm4hep/SimCalorimeterHit.h>
#include <detectors/BTRK/BarrelTrackerSimHit.h>

#include <TDirectory.h>
#include <TCanvas.h>
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>

using namespace fmt;

//------------------
// OccupancyAnalysis (Constructor)
//------------------
OccupancyAnalysis::OccupancyAnalysis(JApplication *app) :
	JEventProcessor(app)
{
}

//------------------
// Init
//------------------
void OccupancyAnalysis::Init()
{
	// Ask service locator a file to write to
	auto file = new TFile("data_work_flow.root");

	// Root related, we switch gDirectory to this file
	file->cd();
	fmt::print("OccupancyAnalysis::gDirectory->pwd()\n");	// >oO Debug print
	gDirectory->pwd();

	// Create a directory for this plugin. And subdirectories for series of histograms
	_dir_main = file->mkdir("data_flow_test");

    // Hits by Z distribution
    _th1_prt_pz = new TH1F("hits_z", "Hits Z distribution [mm]", 100, -30, 30);
    _th1_prt_pz->SetDirectory(_dir_main);

    // Total xy occupancy
    _th2_prt_pxy = new TH2F(NAME_OF(total_occ), "Total XY occupancy (all Z)", 600, -2000, 2000, 300, -2000., 2000.);
    _th2_prt_pxy->SetDirectory(_dir_main);
    _th2_prt_pxy->SetOption("COLSCATZ");		// Draw as heat map by default

}


//------------------
// Process
//------------------
void OccupancyAnalysis::Process(const std::shared_ptr<const JEvent>& event)
{


    fmt::print("OccupancyAnalysis::Process() event {}\n", event->GetEventNumber());
    //auto simhits = event->Get<edm4hep::SimCalorimeterHit>("EcalBarrelHits");
    auto bcal = event->Get<BarrelTrackerSimHit>();
    fmt::print("BCAL {}\n", bcal[0]->getCellID());

//	// Get hits
//	auto hits = event->Get<minimodel::McFluxHit>();
//
// 	for(auto hit: hits) {
//
//		// Create local x,y,z,name as we will use them a lot to drag hit-> around
//		double x = hit->x;
//		double y = hit->y;
//		double z = hit->z;
//
//		th1_hits_z->Fill(z);	// Hits over z axes
//		total_occ->Fill(x, y);	// Total xy occupancy
//		if(z > 0) h_part_occ->Fill(x, y);	// Hadron part xy occupancy (z > 0)
//		if(z <= 0) e_part_occ->Fill(x, y); // electron part xy occupancy (Z < 0)
//
//		// Fill occupancy by layer/vol_name
//		th2_by_layer->Get(hit->vol_name)->Fill(x, y);
//
//		// Fill occupancy per detector
//		auto detector_name = VolNameToDetName(hit->vol_name);	// get detector name
//		if(!detector_name.empty()) {
//			th1_z_by_detector->Get(detector_name)->Fill(z);
//			th2_by_detector->Get(detector_name)->Fill(x, y);
//			th3_by_detector->Get(detector_name)->Fill(z, x, y);  // z, x, y is the right order here
//			th3_hits3d->Fill(z, x, y);
//		}
//	}
}


//------------------
// Finish
//------------------
void OccupancyAnalysis::Finish()
{
	fmt::print("OccupancyAnalysis::Finish() called\n");

	// Next we want to create several pretty canvases (with histograms drawn on "same")
	// But we don't want those canvases to pop up. So we set root to batch mode
	// We will restore the mode afterwards
	//bool save_is_batch = gROOT->IsBatch();
	//gROOT->SetBatch(true);

	// 3D hits distribution
//	auto th3_by_det_canvas = new TCanvas("th3_by_det_cnv", "Occupancy of detectors");
//	dir_main->Append(th3_by_det_canvas);
//	for (auto& kv : th3_by_detector->GetMap()) {
//		auto th3_hist = kv.second;
//		th3_hist->Draw("same");
//	}
//	th3_by_det_canvas->GetPad(0)->BuildLegend();
//
//	// Hits Z by detector
//
//	// Create pretty canvases
//	auto z_by_det_canvas = new TCanvas("z_by_det_cnv", "Hit Z distribution by detector");
//	dir_main->Append(z_by_det_canvas);
//	th1_hits_z->Draw("PLC PFC");
//
//	for (auto& kv : th1_z_by_detector->GetMap()) {
//		auto hist = kv.second;
//		hist->Draw("SAME PLC PFC");
//		hist->SetFillStyle(3001);
//	}
//	z_by_det_canvas->GetPad(0)->BuildLegend();
//
//	gROOT->SetBatch(save_is_batch);
}

