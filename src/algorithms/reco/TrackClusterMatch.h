// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tristan Protzman

#pragma once

#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/TrackClusterMatchCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4hep/Vector3f.h>
#include <cmath>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/reco/TrackClusterMatchConfig.h"

namespace eicrecon {
using TrackClusterMatchAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TrackSegmentCollection, edm4eic::ClusterCollection>,
    algorithms::Output<edm4eic::TrackClusterMatchCollection>>;

class TrackClusterMatch : public TrackClusterMatchAlgorithm,
                          public WithPodConfig<TrackClusterMatchConfig> {
private:
  const algorithms::GeoSvc& m_geo = algorithms::GeoSvc::instance();
  static double distance(const edm4hep::Vector3f& v1, const edm4hep::Vector3f& v2);
  static double Phi_mpi_pi(double phi) { return std::remainder(phi, 2 * M_PI); }

public:
  TrackClusterMatch(std::string_view name)
      : TrackClusterMatchAlgorithm{
            name, {"inputTracks", "inputClusters"}, {"outputParticles"}, ""} {}

  void init() final {};
  void process(const Input&, const Output&) const final;
};
} // namespace eicrecon
