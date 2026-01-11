#include "TrackingEfficiency_processor.h"

#include <Acts/Definitions/TrackParametrization.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/JEvent.h>
#include <JANA/Services/JGlobalRootLock.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/PxPyPzM4D.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <spdlog/logger.h>
#include <Eigen/Core>
#include <any>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "services/log/Log_service.h"
#include "services/rootfile/RootFile_service.h"

//------------------
// Init
//------------------
void TrackingEfficiency_processor::Init() {
  std::string plugin_name = ("tracking_efficiency");

  // Get JANA application
  auto* app = GetApplication();

  // Ask service locator a file to write histograms to
  auto root_file_service = app->GetService<RootFile_service>();

  // Get TDirectory for histograms root file
  auto globalRootLock = app->GetService<JGlobalRootLock>();
  globalRootLock->acquire_write_lock();
  auto* file = root_file_service->GetHistFile();
  globalRootLock->release_lock();

  // Create a directory for this plugin. And subdirectories for series of histograms
  m_dir_main = file->mkdir(plugin_name.c_str());

  // Get logger
  m_log = app->GetService<Log_service>()->logger(plugin_name);
}

//------------------
// Process
//------------------
void TrackingEfficiency_processor::Process(const std::shared_ptr<const JEvent>& event) {
  using namespace ROOT;

  // EXAMPLE I
  // This is access to for final result of the calculation/data transformation of central detector CFKTracking:
  const auto reco_particles =
      event->Get<edm4eic::ReconstructedParticle>("ReconstructedChargedParticles");

  m_log->debug("Tracking reconstructed particles N={}: ", reco_particles.size());
  m_log->debug("   {:<5} {:>8} {:>8} {:>8} {:>8} {:>8}", "[i]", "[px]", "[py]", "[pz]", "[P]",
               "[P*3]");

  for (std::size_t i = 0; i < reco_particles.size(); i++) {
    const auto& particle = *(reco_particles[i]);

    double px = particle.getMomentum().x;
    double py = particle.getMomentum().y;
    double pz = particle.getMomentum().z;
    // ROOT::Math::PxPyPzM4D p4v(px, py, pz, particle.getMass());
    ROOT::Math::Cartesian3D p(px, py, pz);
    m_log->debug("   {:<5} {:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f}", i, px, py, pz, p.R(),
                 p.R() * 3);
  }

  // EXAMPLE II
  // This gets access to more direct ACTS results from CKFTracking
  auto acts_results = event->Get<ActsExamples::ConstTrackContainer>("CentralCKFActsTracks");
  m_log->debug("ACTS Tracks( size: {} )", acts_results.size());
  m_log->debug("{:>10} {:>10}  {:>10} {:>10} {:>10} {:>10} {:>12} {:>12} {:>12} {:>8}", "[loc 0]",
               "[loc 1]", "[phi]", "[theta]", "[q/p]", "[p]", "[err phi]", "[err th]", "[err q/p]",
               "[chi2]");

  // Loop over the tracks
  if (!acts_results.empty()) {
    const auto* track_container = acts_results.front();
    assert(track_container != nullptr && "ConstTrackContainer pointer should not be null");

    for (const auto& track : *track_container) {
      // Get the track parameters
      const auto& parameter  = track.parameters();
      const auto& covariance = track.covariance();
      auto chi2              = track.chi2();
      auto ndf               = track.nDoF();

      m_log->debug("{:>10.2f} {:>10.2f}  {:>10.2f} {:>10.3f} {:>10.4f} {:>10.3f} {:>12.4e} "
                   "{:>12.4e} {:>12.4e} {:>8.2f} {:<6}",
                   parameter[Acts::eBoundLoc0], parameter[Acts::eBoundLoc1],
                   parameter[Acts::eBoundPhi], parameter[Acts::eBoundTheta],
                   parameter[Acts::eBoundQOverP], 1.0 / parameter[Acts::eBoundQOverP],
                   sqrt(covariance(Acts::eBoundPhi, Acts::eBoundPhi)),
                   sqrt(covariance(Acts::eBoundTheta, Acts::eBoundTheta)),
                   sqrt(covariance(Acts::eBoundQOverP, Acts::eBoundQOverP)), chi2, ndf);
    }
  }

  // EXAMPLE III
  // Loop over MC particles
  auto mc_particles = event->Get<edm4hep::MCParticle>("MCParticles");
  m_log->debug("MC particles N={}: ", mc_particles.size());
  m_log->debug("   {:<5} {:<6} {:<7} {:>8} {:>8} {:>8} {:>8}", "[i]", "status", "[PDG]", "[px]",
               "[py]", "[pz]", "[P]");
  for (std::size_t i = 0; i < mc_particles.size(); i++) {
    const auto* particle = mc_particles[i];

    // GeneratorStatus() == 1 - stable particles from MC generator. 0 - might be added by Geant4
    if (particle->getGeneratorStatus() != 1) {
      continue;
    }

    double px = particle->getMomentum().x;
    double py = particle->getMomentum().y;
    double pz = particle->getMomentum().z;
    ROOT::Math::PxPyPzM4D p4v(px, py, pz, particle->getMass());
    ROOT::Math::Cartesian3D p(px, py, pz);
    if (p.R() < 1) {
      continue;
    }

    m_log->debug("   {:<5} {:<6} {:<7} {:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f}", i,
                 particle->getGeneratorStatus(), particle->getPDG(), px, py, pz, p.R());
  }
}

//------------------
// Finish
//------------------
void TrackingEfficiency_processor::Finish() {
  fmt::print("OccupancyAnalysis::Finish() called\n");

  // Next we want to create several pretty canvases (with histograms drawn on "same")
  // But we don't want those canvases to pop up. So we set root to batch mode
  // We will restore the mode afterwards
  //bool save_is_batch = gROOT->IsBatch();
  //gROOT->SetBatch(true);

  // 3D hits distribution
  //      auto th3_by_det_canvas = new TCanvas("th3_by_det_cnv", "Occupancy of detectors");
  //      dir_main->Append(th3_by_det_canvas);
  //      for (auto& kv : th3_by_detector->GetMap()) {
  //              auto th3_hist = kv.second;
  //              th3_hist->Draw("same");
  //      }
  //      th3_by_det_canvas->GetPad(0)->BuildLegend();
  //
  //      // Hits Z by detector
  //
  //      // Create pretty canvases
  //      auto z_by_det_canvas = new TCanvas("z_by_det_cnv", "Hit Z distribution by detector");
  //      dir_main->Append(z_by_det_canvas);
  //      th1_hits_z->Draw("PLC PFC");
  //
  //      for (auto& kv : th1_z_by_detector->GetMap()) {
  //              auto hist = kv.second;
  //              hist->Draw("SAME PLC PFC");
  //              hist->SetFillStyle(3001);
  //      }
  //      z_by_det_canvas->GetPad(0)->BuildLegend();
  //
  //      gROOT->SetBatch(save_is_batch);
}
