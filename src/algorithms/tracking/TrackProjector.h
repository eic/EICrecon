// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Dmitry Romanov, Dmitry Kalinkin

#pragma once

#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Utilities/UnitVectors.hpp>
#include <algorithms/algorithm.h>
#include <edm4eic/Cov2f.h>
#include <edm4eic/Cov3f.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <math.h>
#include <memory>
#include <string>
#include <string_view>
#include <valarray>
#include <vector>

#include "algorithms/tracking/ActsExamplesEdm.h"
#include "algorithms/tracking/ActsGeometryProvider.h"

namespace eicrecon {

template <typename edm_t = eicrecon::ActsExamplesEdm>
using TrackProjectorAlgorithm = algorithms::Algorithm<
    algorithms::Input<std::vector<typename edm_t::Trajectories>, edm4eic::TrackCollection>,
    algorithms::Output<edm4eic::TrackSegmentCollection>>;

template <typename edm_t = eicrecon::ActsExamplesEdm>
class TrackProjector : public TrackProjectorAlgorithm<edm_t> {
public:
  TrackProjector(std::string_view name)
      : TrackProjectorAlgorithm<edm_t>{name,
                                       {"inputActsTrajectories"},
                                       {"outputTrackSegments"},
                                       "Exports track states as segments"} {}

  void init() final;
  void process(const typename TrackProjector<edm_t>::Input&,
               const typename TrackProjector<edm_t>::Output&) const final;

private:
  std::shared_ptr<const ActsGeometryProvider> m_geo_provider;
};

} // namespace eicrecon
