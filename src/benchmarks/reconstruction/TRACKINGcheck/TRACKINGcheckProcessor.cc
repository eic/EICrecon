
//
// Template for this file generated with eicmkplugin.py
//

#include <Acts/Geometry/GeometryContext.hpp>
#include <ActsExamples/EventData/Trajectories.hpp>
#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/JEvent.h>
#include <TDirectory.h>
#include <memory>
#include <vector>

#include "TRACKINGcheckProcessor.h"
#include "services/rootfile/RootFile_service.h"

//-------------------------------------------
// InitWithGlobalRootLock
//-------------------------------------------
void TRACKINGcheckProcessor::InitWithGlobalRootLock() {

  auto rootfile_svc = GetApplication()->GetService<RootFile_service>();
  auto* rootfile    = rootfile_svc->GetHistFile();
  rootfile->mkdir("TRACKING")->cd();

  hist1D["Trajectories_trajectories_per_event"] =
      new TH1I("Trajectories_trajectories_per_event",
               "TRACKING Reconstructed trajectories/event;Ntrajectories", 201, -0.5, 200.5);
  hist1D["Trajectories_time"] = new TH1D(
      "Trajectories_time", "TRACKING reconstructed particle time;time (ns)", 200, -100.0, 100.0);
  hist2D["Trajectories_xy"] =
      new TH2D("Trajectories_xy", "TRACKING reconstructed position Y vs. X;x;y", 100, -1000.0,
               1000.0, 100, -1000., 1000.0);
  hist1D["Trajectories_z"] =
      new TH1D("Trajectories_z", "TRACKING reconstructed position Z;z", 200, -50.0, 50.0);

  // Set some draw options
  hist2D["Trajectories_xy"]->SetOption("colz");
}

//-------------------------------------------
// ProcessSequential
//-------------------------------------------
void TRACKINGcheckProcessor::ProcessSequential(const std::shared_ptr<const JEvent>& event) {
  auto Trajectories = event->Get<ActsExamples::Trajectories>("CentralCKFActsTrajectories");

  // Fill histograms here

  // Trajectories
  hist1D["Trajectories_trajectories_per_event"]->Fill(Trajectories.size());

  for (const auto* traj : Trajectories) {
    for (auto entryIndex : traj->tips()) {
      if (!traj->hasTrackParameters(entryIndex)) {
        continue;
      }
      auto trackparams = traj->trackParameters(entryIndex);

      auto pos = trackparams.position(Acts::GeometryContext());
      auto t   = trackparams.time();

      hist1D["Trajectories_time"]->Fill(t);
      hist2D["Trajectories_xy"]->Fill(pos.x(), pos.y());
      hist1D["Trajectories_z"]->Fill(pos.z());
    }
  }
}

//-------------------------------------------
// FinishWithGlobalRootLock
//-------------------------------------------
void TRACKINGcheckProcessor::FinishWithGlobalRootLock() {

  // Do any final calculations here.
}
