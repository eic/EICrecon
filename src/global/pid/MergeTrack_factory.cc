// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "MergeTrack_factory.h"

//-----------------------------------------------------------------------------
void eicrecon::MergeTrack_factory::Init() {

  // get plugin name and tag
  auto app = GetApplication();
  auto detector_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");
  auto param_prefix = detector_name + ":" + GetTag();
  InitDataTags(param_prefix);

  // services
  InitLogger(param_prefix, "info");
  m_algo.AlgorithmInit(m_log);
  m_log->debug("detector_name='{}'  param_prefix='{}'", detector_name, param_prefix);

}

//-----------------------------------------------------------------------------
void eicrecon::MergeTrack_factory::BeginRun(const std::shared_ptr<const JEvent> &event) {
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
    SetCollection(std::move(out_track_collection));
  }
  catch(std::exception &e) {
    m_log->warn("Exception in underlying algorithm: {}. Event data will be skipped", e.what());
  }

}
