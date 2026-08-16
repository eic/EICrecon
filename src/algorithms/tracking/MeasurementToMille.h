// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 ePIC Collaboration

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/AlignmentDerivativeSetCollection.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackCollection.h>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include "ActsGeometryProvider.h"
#include "MeasurementToMilleConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using MeasurementToMilleAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TrackCollection, edm4eic::Measurement2DCollection>,
    algorithms::Output<edm4eic::AlignmentDerivativeSetCollection>>;

/// Converts reconstructed tracks and measurements into Millepede-II alignment
/// derivative sets for silicon tracker alignment.
///
/// For each track passing quality cuts the algorithm iterates over the
/// Measurement2D hits associated with the track, looks up the alignment layer
/// from the ACTS geometry, and fills one AlignmentDerivativeSet per
/// (track, surface) pair with approximate local and global derivatives.
///
/// NOTE: The current implementation uses simplified (approximate) derivatives.
/// The correct derivatives require calling Acts::detail::makeTrackAlignmentState()
/// on the ACTS track states. Once the ACTS Alignment kernel is accessible from
/// EICrecon, replace the simplified code with the proper computation.
class MeasurementToMille : public MeasurementToMilleAlgorithm,
                           public WithPodConfig<MeasurementToMilleConfig> {
public:
  MeasurementToMille(std::string_view name)
      : MeasurementToMilleAlgorithm{
            name,
            {"inputTracks", "inputMeasurements"},
            {"outputAlignmentDerivatives"},
            "Fills Millepede-II alignment derivatives for silicon tracker"} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  /// Map from Acts::GeometryIdentifier (as uint64_t) to 0-based silicon layer index.
  /// Built once during init() by walking the ACTS TrackingGeometry.
  std::unordered_map<std::uint64_t, int> m_surfaceToLayer;

  std::shared_ptr<const ActsGeometryProvider> m_geo;
};

} // namespace eicrecon
