// Copyright (C) 2022, 2023 Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <Acts/Surfaces/Surface.hpp>
#include <JANA/JEvent.h>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// algorithms
#include "algorithms/tracking/ActsExamplesEdm.h"
#include "algorithms/tracking/ActsPodioEdm.h"
#include "algorithms/tracking/TrackPropagation.h"
#include "algorithms/tracking/TrackPropagationConfig.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/geometry/acts/ACTSGeo_service.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

namespace eicrecon {

template <typename edm_t = eicrecon::ActsExamplesEdm>
class RichTrack_factory : public JOmniFactory<RichTrack_factory<edm_t>, TrackPropagationConfig> {

private:
  using AlgoT    = eicrecon::TrackPropagation<edm_t>;
  using FactoryT = JOmniFactory<RichTrack_factory<edm_t>, TrackPropagationConfig>;

  std::unique_ptr<AlgoT> m_algo;

  template <typename T> using PodioInput   = typename FactoryT::template PodioInput<T>;
  template <typename T> using PodioOutput  = typename FactoryT::template PodioOutput<T>;
  template <typename T> using Input        = typename FactoryT::template Input<T>;
  template <typename T> using Output       = typename FactoryT::template Output<T>;
  template <typename T> using ParameterRef = typename FactoryT::template ParameterRef<T>;
  template <typename T> using Service      = typename FactoryT::template Service<T>;

  PodioInput<edm4eic::Track> m_tracks_input{this};
  Input<typename edm_t::Trajectories> m_acts_trajectories_input{this};
  Input<typename edm_t::ConstTrackContainer> m_acts_tracks_input{this};
  PodioOutput<edm4eic::TrackSegment> m_track_segments_output{this};

  Service<DD4hep_service> m_GeoSvc{this};
  Service<ACTSGeo_service> m_ACTSGeoSvc{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>();
    m_algo->applyConfig(this->config());

    if (this->config().filter_surfaces.empty())
      throw JException("cannot find filter surface for RICH track propagation");

    m_algo->init(m_GeoSvc().detector(), m_ACTSGeoSvc().actsGeoProvider(), this->logger());
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->propagateToSurfaceList(
        {*m_tracks_input(), m_acts_trajectories_input(), m_acts_tracks_input()},
        {m_track_segments_output().get()});
  }
};

} // namespace eicrecon
