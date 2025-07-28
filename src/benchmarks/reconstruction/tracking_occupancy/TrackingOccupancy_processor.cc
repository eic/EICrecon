#include "TrackingOccupancy_processor.h"

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Services/JGlobalRootLock.h>
#include <TDirectory.h>
#include <string>

#include "benchmarks/reconstruction/tracking_occupancy/HitReconstructionAnalysis.h"
#include "benchmarks/reconstruction/tracking_occupancy/TrackingOccupancyAnalysis.h"
#include "services/log/Log_service.h"
#include "services/rootfile/RootFile_service.h"

//------------------
// Init
//------------------
void TrackingOccupancy_processor::Init() {
  std::string plugin_name = ("tracking_occupancy");

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

  // Occupancy analysis
  m_occupancy_analysis.init(app, m_dir_main);
  m_hit_reco_analysis.init(app, m_dir_main);

  // Get logger
  m_log = app->GetService<Log_service>()->logger(plugin_name);
}

//------------------
// Process
//------------------
void TrackingOccupancy_processor::Process(const std::shared_ptr<const JEvent>& event) {
  // Process raw hits from DD4Hep
  m_occupancy_analysis.process(event);

  // Process hits reconstructed with
  m_hit_reco_analysis.process(event);
}

//------------------
// Finish
//------------------
void TrackingOccupancy_processor::Finish() {
  // Nothing to do here
}
