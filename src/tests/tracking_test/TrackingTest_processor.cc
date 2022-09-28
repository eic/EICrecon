
#include "TrackingTest_processor.h"
#include "algorithms/tracking/JugTrack/Trajectories.hpp"
#include "extensions/spdlog/SpdlogExtensions.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <fmt/core.h>

#include <TDirectory.h>
#include <TCanvas.h>
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <Math/LorentzVector.h>
#include <Math/GenVector/PxPyPzM4D.h>

#include <spdlog/spdlog.h>

#include <edm4hep/SimCalorimeterHit.h>
#include <edm4hep/MCParticle.h>
#include <edm4eic/TrackerHit.h>
#include <edm4eic/TrackParameters.h>
#include <edm4eic/ReconstructedParticle.h>

#include <algorithms/tracking/TrackerSourceLinkerResult.h>
#include <algorithms/tracking/ParticlesFromTrackFitResult.h>
#include <algorithms/tracking/JugTrack/Track.hpp>
#include <services/rootfile/RootFile_service.h>

using namespace fmt;

//------------------
// OccupancyAnalysis (Constructor)
//------------------
TrackingTest_processor::TrackingTest_processor(JApplication *app) :
	JEventProcessor(app)
{
}

//------------------
// Init
//------------------
void TrackingTest_processor::Init()
{
    std::string plugin_name=("tracking_test");

    // Get JANA application
    auto app = GetApplication();

    // Ask service locator a file to write histograms to
    auto root_file_service = app->GetService<RootFile_service>();

    // Get TDirectory for histograms root file
    auto globalRootLock = app->GetService<JGlobalRootLock>();
    globalRootLock->acquire_write_lock();
    auto file = root_file_service->GetHistFile();
    globalRootLock->release_lock();

    // Create a directory for this plugin. And subdirectories for series of histograms
    m_dir_main = file->mkdir(plugin_name.c_str());

    // Get log level from user parameter or default
    std::string log_level_str = "info";
    m_log = app->GetService<Log_service>()->logger(plugin_name);
    app->SetDefaultParameter(plugin_name + ":LogLevel", log_level_str, "LogLevel: trace, debug, info, warn, err, critical, off");
    m_log->set_level(eicrecon::ParseLogLevel(log_level_str));
    for(auto pair: app->GetJParameterManager()->GetAllParameters()) {
        m_log->info("{:<20} | {}", pair.first, pair.second->GetDescription());
    }
}


//------------------
// Process
//------------------
void TrackingTest_processor::Process(const std::shared_ptr<const JEvent>& event)
{
    using namespace ROOT;


//
//    fmt::print("OccupancyAnalysis::Process() event {}\n", event->GetEventNumber());
//
//    //auto simhits = event->Get<edm4hep::SimCalorimeterHit>("EcalBarrelHits");
//    //auto raw_hits = event->Get<edm4eic::RawTrackerHit>("BarrelTrackerRawHit");
//
//    auto hits = event->Get<edm4eic::TrackerHit>("BarrelTrackerHit");
//
    //auto result = event->GetSingle<eicrecon::TrackerSourceLinkerResult>("CentralTrackerSourceLinker");
//    spdlog::info("Result counts sourceLinks.size()={} measurements.size()={}", result->sourceLinks->size(), result->measurements->size());
//
//    auto truth_init = event->Get<Jug::TrackParameters>("");
//    spdlog::info("truth_init.size()={}", truth_init.size());
//
    //auto trajectories = event->Get<Jug::Trajectories>("Trajectories");

//    fmt::print("BCAL {}\n", bcal[0]->getCellID());

    auto trk_result = event->GetSingle<ParticlesFromTrackFitResult>("CentralTrackingParticles");


    m_log->debug("Tracking reconstructed particles N={}: ", trk_result->particles()->size());
    m_log->debug("   {:<5} {:>8} {:>8} {:>8} {:>8} {:>8}","[i]", "[px]", "[py]", "[pz]", "[P]", "[P*3]");


    auto reco_particles = trk_result->particles();

    for(size_t i=0; i < reco_particles->size(); i++) {
        auto particle = (*reco_particles)[i];

        double px = particle.getMomentum().x;
        double py = particle.getMomentum().y;
        double pz = particle.getMomentum().z;
        // ROOT::Math::PxPyPzM4D p4v(px, py, pz, particle.getMass());
        ROOT::Math::Cartesian3D p(px, py, pz);
        m_log->debug("   {:<5} {:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f}", i,  px, py, pz, p.R(), p.R()*3);
    }

    auto mc_particles = event->Get<edm4hep::MCParticle>("MCParticles");
//    fmt::print("OccupancyAnalysis::Process() mc_particles N {}\n", mc_particles.size());
//

    auto particles = event->GetSingle<edm4eic::ReconstructedParticle>("ReconstructedParticles");
    auto track_params = event->GetSingle<edm4eic::TrackParameters>("TrackParameters");

    m_log->debug("MC particles N={}: ", mc_particles.size());
    m_log->debug("   {:<5} {:<6} {:<7} {:>8} {:>8} {:>8} {:>8}","[i]", "status", "[PDG]",  "[px]", "[py]", "[pz]", "[P]");
    for(size_t i=0; i < mc_particles.size(); i++) {

        auto particle=mc_particles[i];

        if(particle->getGeneratorStatus() != 1) continue;
//
        double px = particle->getMomentum().x;
        double py = particle->getMomentum().y;
        double pz = particle->getMomentum().z;
        ROOT::Math::PxPyPzM4D p4v(px, py, pz, particle->getMass());
        ROOT::Math::Cartesian3D p(px, py, pz);
        if(p.R()<1) continue;

        m_log->debug("   {:<5} {:<6} {:<7} {:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f}", i, particle->getGeneratorStatus(), particle->getPDG(),  px, py, pz, p.R());

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
    }

//    for(auto& trajectory: trajectories) {
//        m_log->debug("Trajectory empty {}", trajectory->empty());
//        m_log->debug("Trajectory multiTrajectory size {}", trajectory->multiTrajectory().size());
//        m_log->debug("Trajectory trajectory->trackParameters(0).momentum().x() {}", trajectory->trackParameters(0).momentum().x());
//    }


auto hits = event->Get<edm4eic::TrackerHit>("trackerHits");





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
void TrackingTest_processor::Finish()
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

