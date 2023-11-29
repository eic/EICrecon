// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <ActsExamples/EventData/Track.hpp>
#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/JFactoryT.h>
#include <spdlog/logger.h>
#include <exception>
#include <map>

#include "ActsExamples/EventData/Trajectories.hpp"
#include "IterativeVertexFinder.h"
#include "IterativeVertexFinder_factory.h"
#include "services/geometry/acts/ACTSGeo_service.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "services/io/podio/JFactoryPodioT.h"

void eicrecon::IterativeVertexFinder_factory::Init() {
  auto *app = GetApplication();

  // This prefix will be used for parameters
  std::string plugin_name  = GetPluginName();
  std::string param_prefix = plugin_name + ":" + GetTag();

  // Initialize input tags
  InitDataTags(param_prefix);

  // Initialize logger
  InitLogger(app, param_prefix, "info");

  // Get ACTS context from ACTSGeo service
  auto acts_service  = app->GetService<ACTSGeo_service>();
  auto dd4hp_service = app->GetService<DD4hep_service>();

  // Algorithm configuration
  auto cfg = GetDefaultConfig();

  app->SetDefaultParameter(param_prefix + ":maxVertices", cfg.m_maxVertices,
                           "Maximum num vertices that can be found");
  app->SetDefaultParameter(param_prefix + ":reassignTracksAfterFirstFit",
                           cfg.m_reassignTracksAfterFirstFit,
                           "Whether or not to reassign tracks after first fit");

  // Initialize algorithm
  m_vertexing_algo.applyConfig(cfg);
  m_vertexing_algo.init(acts_service->actsGeoProvider(), m_log);
}

void eicrecon::IterativeVertexFinder_factory::ChangeRun(
    const std::shared_ptr<const JEvent>& event) {
  JFactoryT::ChangeRun(event);
}

void eicrecon::IterativeVertexFinder_factory::Process(const std::shared_ptr<const JEvent>& event) {

  auto trajectories = event->Get<ActsExamples::Trajectories>(GetInputTags()[0]);
  auto tracks       = event->Get<ActsExamples::ConstTrackContainer>(GetInputTags()[1]);

  m_log->debug("Process method");

  try {
    auto vertices = m_vertexing_algo.produce(trajectories);
    SetCollection(std::move(vertices));
  } catch (std::exception& e) {
    throw JException(e.what());
  }
}
