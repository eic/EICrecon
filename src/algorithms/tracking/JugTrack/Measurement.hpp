// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck

#pragma once

#include "Acts/EventData/Measurement.hpp"
#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/SourceLink.hpp"
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "IndexSourceLink.hpp"

#include <cassert>
#include <vector>
#include <spdlog/spdlog.h>

namespace eicrecon {

  /// Variable measurement type that can contain all possible combinations.
  using Measurement = ::Acts::BoundVariantMeasurement;
  /// Container of measurements.
  ///
  /// In contrast to the source links, the measurements themself must not be
  /// orderable. The source links stored in the measurements are treated
  /// as opaque here and no ordering is enforced on the stored measurements.
  using MeasurementContainer = std::vector<Measurement>;

  /// Calibrator to convert an index source link to a measurement.
  class MeasurementCalibrator {
  public:
    /// Construct an invalid calibrator. Required to allow copying.
    MeasurementCalibrator() = default;
    /// Construct using a user-provided container to chose measurements from.
    MeasurementCalibrator(const MeasurementContainer& measurements) : m_measurements(&measurements) {}

    /// Find the measurement corresponding to the source link.
    ///
    /// @tparam parameters_t Track parameters type
    /// @param gctx The geometry context (unused)
    /// @param trackState The track state to calibrate
    void calibrate(
        const Acts::GeometryContext& /*gctx*/,
        Acts::MultiTrajectory<Acts::VectorMultiTrajectory>::TrackStateProxy
            trackState) const {
      const auto& sourceLink =
          static_cast<const IndexSourceLink&>(trackState.uncalibrated());

      std::visit(
          [&trackState](const auto& meas) { trackState.setCalibrated(meas); },
          (*m_measurements)[sourceLink.index()]);
    }

  private:
    // use pointer so the calibrator is copyable and default constructible.
    const MeasurementContainer* m_measurements = nullptr;
  };

} // namespace Jug

