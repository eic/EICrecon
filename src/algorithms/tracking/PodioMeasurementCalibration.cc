// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck
// Based on ActsExamples/EventData/MeasurementCalibration.cpp from the ACTS project

#include "PodioMeasurementCalibration.h"

#include <Acts/EventData/SourceLink.hpp>
#include <ActsExamples/EventData/IndexSourceLink.hpp>
#include <ActsExamples/EventData/Measurement.hpp>

#include <cassert>

namespace eicrecon {

void PodioPassThroughCalibrator::calibrate(
    const ActsExamples::MeasurementContainer& measurements,
    const ActsExamples::ClusterContainer* /*clusters*/, const Acts::GeometryContext& /*gctx*/,
    const Acts::CalibrationContext& /*cctx*/, const Acts::SourceLink& sourceLink,
    ActsPlugins::MutablePodioTrackStateContainer<Acts::RefHolder>::TrackStateProxy& trackState)
    const {

  trackState.setUncalibratedSourceLink(Acts::SourceLink{sourceLink});

  // Extract the measurement index from the source link
  // In both Acts < 39 and Acts >= 39, IndexSourceLink has an index() method
  const ActsExamples::IndexSourceLink& idxSourceLink =
      sourceLink.get<ActsExamples::IndexSourceLink>();
  std::size_t index = idxSourceLink.index();

  assert((index < measurements.size()) && "Source link index is outside the container bounds");

  const ActsExamples::ConstVariableBoundMeasurementProxy measurement =
      measurements.getMeasurement(index);

  Acts::visit_measurement(measurement.size(), [&](auto N) -> void {
    constexpr std::size_t kMeasurementSize = decltype(N)::value;
    const ActsExamples::ConstFixedBoundMeasurementProxy<kMeasurementSize> fixedMeasurement =
        static_cast<ActsExamples::ConstFixedBoundMeasurementProxy<kMeasurementSize>>(measurement);

    trackState.allocateCalibrated(fixedMeasurement.parameters().eval(),
                                  fixedMeasurement.covariance().eval());
    trackState.setProjectorSubspaceIndices(fixedMeasurement.subspaceIndices());
  });
}

PodioMeasurementCalibratorAdapter::PodioMeasurementCalibratorAdapter(
    const PodioMeasurementCalibrator& calibrator,
    const ActsExamples::MeasurementContainer& measurements,
    const ActsExamples::ClusterContainer* clusters)
    : m_calibrator{calibrator}, m_measurements{measurements}, m_clusters{clusters} {}

void PodioMeasurementCalibratorAdapter::calibrate(
    const Acts::GeometryContext& gctx, const Acts::CalibrationContext& cctx,
    const Acts::SourceLink& sourceLink,
    ActsPlugins::MutablePodioTrackStateContainer<Acts::RefHolder>::TrackStateProxy trackState)
    const {
  return m_calibrator.calibrate(m_measurements, m_clusters, gctx, cctx, sourceLink, trackState);
}

} // namespace eicrecon
