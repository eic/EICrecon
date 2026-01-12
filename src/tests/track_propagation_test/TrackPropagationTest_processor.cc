
#include <Acts/Definitions/Algebra.hpp>
#include <Acts/EventData/TrackContainer.hpp>
#include <Acts/EventData/TrackProxy.hpp>
#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#include <Acts/Surfaces/DiscSurface.hpp>
#include <Acts/Surfaces/RadialBounds.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/JEvent.h>
#include <JANA/JException.h>
#include <JANA/Services/JGlobalRootLock.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackPoint.h>
#include <edm4hep/Vector3f.h>
#include <fmt/format.h>
#include <spdlog/logger.h>
#include <Eigen/Geometry>
#include <exception>
#include <gsl/pointers>
#include <map>
#include <string>
#include <vector>

#include "TrackPropagation.h"
#include "TrackPropagationTest_processor.h"
#include "services/geometry/acts/ACTSGeo_service.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "services/rootfile/RootFile_service.h"

//------------------
// Init
//------------------
void TrackPropagationTest_processor::Init() {
  std::string plugin_name = ("track_propagation_test");

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

  // Get log level from user parameter or default
  InitLogger(app, plugin_name);

  auto dd4hep_service = GetApplication()->GetService<DD4hep_service>();
  auto acts_service   = GetApplication()->GetService<ACTSGeo_service>();

  m_propagation_algo.setDetector(dd4hep_service->detector());
  m_propagation_algo.setGeometryService(acts_service->actsGeoProvider());
  m_propagation_algo.init();

  // Create HCal surface that will be used for propagation
  auto transform = Acts::Transform3::Identity();

  // make a reference disk to mimic electron-endcap HCal
  const auto hcalEndcapNZ    = -3322.;
  const auto hcalEndcapNMinR = 83.01;
  const auto hcalEndcapNMaxR = 950;
  auto hcalEndcapNBounds = std::make_shared<Acts::RadialBounds>(hcalEndcapNMinR, hcalEndcapNMaxR);
  auto hcalEndcapNTrf    = transform * Acts::Translation3(Acts::Vector3(0, 0, hcalEndcapNZ));
  m_hcal_surface = Acts::Surface::makeShared<Acts::DiscSurface>(hcalEndcapNTrf, hcalEndcapNBounds);
}

//------------------
// Process
//------------------
// This function is called every event
void TrackPropagationTest_processor::Process(const std::shared_ptr<const JEvent>& event) {
  m_log->trace("TrackPropagationTest_processor event");

  // Get track containers from tracking
  auto track_states = event->Get<Acts::ConstVectorMultiTrajectory>("CentralCKFActsTrackStates");
  auto tracks       = event->Get<Acts::ConstVectorTrackContainer>("CentralCKFActsTracks");

  if (track_states.empty() || tracks.empty()) {
    m_log->debug("No track containers found");
    return;
  }

  // Construct ConstTrackContainer from underlying containers
  auto trackStateContainer =
      std::make_shared<Acts::ConstVectorMultiTrajectory>(*track_states.front());
  auto trackContainer = std::make_shared<Acts::ConstVectorTrackContainer>(*tracks.front());
  ActsExamples::ConstTrackContainer track_container(trackContainer, trackStateContainer);

  // Iterate over tracks
  m_log->debug("Propagating through {} tracks", track_container.size());
  for (const auto& track : track_container) {
    m_log->trace(" -- track {} --", track.index());

    std::unique_ptr<edm4eic::TrackPoint> projection_point;
    try {
      // >>> try to propagate to surface <<<
      projection_point =
          m_propagation_algo.propagate(edm4eic::Track{}, track, track_container, m_hcal_surface);
    } catch (std::exception& e) {
      throw JException(e.what());
    }

    if (!projection_point) {
      m_log->trace("   could not propagate track {}!", track.index());
      continue;
    }

    // Now go through reconstructed tracks points

    auto pos    = projection_point->position;
    auto length = projection_point->pathlength;
    m_log->trace("   {:>10} {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f}", track.index(), pos.x, pos.y,
                 pos.z, length);
  }
}

//------------------
// Finish
//------------------
void TrackPropagationTest_processor::Finish() {
  //    m_log->trace("TrackPropagationTest_processor finished\n");
}
