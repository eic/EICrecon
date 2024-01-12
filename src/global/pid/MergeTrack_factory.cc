// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "MergeTrack_factory.h"

#include <JANA/JException.h>
#include <fmt/core.h>
#include <spdlog/logger.h>
#include <exception>

//-----------------------------------------------------------------------------
void eicrecon::MergeTrack_factory::Init() {

  // get plugin name and tag
  auto *app    = GetApplication();
  auto plugin = GetPluginName();
  auto prefix = plugin + ":" + GetTag();

  // services
  InitLogger(app, prefix, "info");
  m_algo.AlgorithmInit(m_log);
  m_log->debug("MergeTrack_factory: plugin='{}' prefix='{}'", plugin, prefix);

}

//-----------------------------------------------------------------------------
void eicrecon::MergeTrack_factory::Process(const std::shared_ptr<const JEvent> &event) {

  // get input collections
  std::vector<const edm4eic::TrackSegmentCollection*> in_track_collections;
  for(auto& input_tag : GetInputTags())
    in_track_collections.push_back(
        static_cast<const edm4eic::TrackSegmentCollection*>(event->GetCollectionBase(input_tag))
        );

  // call the MergeTracks algorithm
  try {
    auto out_track_collection = m_algo.AlgorithmProcess(in_track_collections);
    SetCollection<edm4eic::TrackSegment>(GetOutputTags()[0], std::move(out_track_collection));
  }
  catch(std::exception &e) {
    throw JException(e.what());
  }

}
