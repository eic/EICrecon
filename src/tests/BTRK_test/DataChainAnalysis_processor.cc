#include "DataChainAnalysis_processor.h"
#include "algorithms/tracking/JugTrack/Trajectories.hpp"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <fmt/core.h>

#include <edm4hep/SimCalorimeterHit.h>
#include <edm4hep/MCParticle.h>
#include <eicd/TrackerHit.h>

#include <TDirectory.h>
#include <TCanvas.h>
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <Math/LorentzVector.h>
#include <Math/GenVector/PxPyPzM4D.h>

#include <spdlog/spdlog.h>
#include <eicd/TrackParameters.h>

#include <algorithms/tracking/TrackerSourceLinkerResult.h>
#include <algorithms/tracking/JugTrack/Track.hpp>


using namespace fmt;

//------------------
// OccupancyAnalysis (Constructor)
//------------------
DataChainAnalysis_processor::DataChainAnalysis_processor(JApplication *app) :
	JEventProcessor(app)
{
}

//------------------
// Init
//------------------
void DataChainAnalysis_processor::Init()
{
	// Ask service locator a file to write to

	// Root related, we switch gDirectory to this file
    if(!gFile)
    {
        auto file = new TFile("data_work_flow.root");
    }
	//file->cd();
	fmt::print("OccupancyAnalysis::gDirectory->pwd()\n");	// >oO Debug print
	gDirectory->pwd();

	// Create a directory for this plugin. And subdirectories for series of histograms
	m_dir_main = gFile->mkdir("BTRK_test");

    // Hits by Z distribution
    m_th1_prt_pz = new TH1F("prt_pz", "MCParticles Pz distribution [GeV]", 100, 0, 30);
    m_th1_prt_pz->SetDirectory(m_dir_main);

    // Stable particle energy distribution
    m_th1_prt_energy = new TH1F("prt_energy", "MCParticles E distribution [GeV]", 100, 0, 30);
    m_th1_prt_energy->SetDirectory(m_dir_main);

    // Hits P Theta distribution
    m_th1_prt_theta = new TH1F("prt_theta", "MCParticles Theta [deg]", 100, -7, 7);
    m_th1_prt_theta->SetDirectory(m_dir_main);

    // Hits P Phi distribution
    m_th1_prt_phi = new TH1F("prt_phi", "MCParticles Phi [deg]", 100, -7, 7);
    m_th1_prt_phi->SetDirectory(m_dir_main);

    // Total xy occupancy
    m_th2_prt_pxy = new TH2F("prt_pxpy", "MCParticles Px vs Py", 300, 0, 30, 300, 0, 30);
    m_th2_prt_pxy->SetDirectory(m_dir_main);
    m_th2_prt_pxy->SetOption("COLSCATZ");		// Draw as heat map by default
}


//------------------
// Process
//------------------
void DataChainAnalysis_processor::Process(const std::shared_ptr<const JEvent>& event)
{
    using namespace ROOT;

    fmt::print("OccupancyAnalysis::Process() event {}\n", event->GetEventNumber());

    //auto simhits = event->Get<edm4hep::SimCalorimeterHit>("EcalBarrelHits");
    //auto raw_hits = event->Get<eicd::RawTrackerHit>("BarrelTrackerRawHit");

    auto hits = event->Get<eicd::TrackerHit>("BarrelTrackerHit");

    auto result = event->GetSingle<eicrecon::TrackerSourceLinkerResult>("TrackerSourceLinkerResult");
    spdlog::info("Result counts sourceLinks.size()={} measurements.size()={}", result->sourceLinks.size(), result->measurements.size());

    auto truth_init = event->Get<Jug::TrackParameters>("");
    spdlog::info("truth_init.size()={}", truth_init.size());

    auto trajectories = event->Get<Jug::Trajectories>("");

//    fmt::print("BCAL {}\n", bcal[0]->getCellID());
//
//    auto particles = event->Get<edm4hep::MCParticle>("MCParticles");
//    fmt::print("OccupancyAnalysis::Process() particles N {}\n", particles.size());
//
//    for(auto& particle: particles) {
//        if(particle->getGeneratorStatus() != 1) continue;
//
//        fmt::print("OccupancyAnalysis::Process() stable: {}\n", particle->getPDG());
//
//        double px = particle->getMomentum().x;
//        double py = particle->getMomentum().y;
//        double pz = particle->getMomentum().z;
//        ROOT::Math::PxPyPzM4D p4v(px, py, pz, particle->getMass());
//        ROOT::Math::Cartesian3D p(px, py, pz);
//        fmt::print("OccupancyAnalysis::Process() pz: {}\n", pz);
//
//        m_th1_prt_pz->Fill(pz);
//        m_th2_prt_pxy->Fill(px, py);
//
//        m_th1_prt_theta->Fill(p.Theta());
//        if(pz>0) {
//            m_th1_prt_phi->Fill(p.Phi());
//        } else {
//            m_th1_prt_phi->Fill(-p.Phi());
//        }
//
//        m_th1_prt_energy->Fill(p4v.E());
//
//
//        /*
//        fmt::print("OccupancyAnalysis::Process() theta  : {}\n", p.Theta());
//        fmt::print("OccupancyAnalysis::Process() theta2 : {}\n", acos(pz/p.R()));
//        fmt::print("OccupancyAnalysis::Process() phi    : {}\n", p.Phi());
//        fmt::print("OccupancyAnalysis::Process() phi    : {}\n", atan(py/px));
//         */
//    }




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
void DataChainAnalysis_processor::Finish()
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

