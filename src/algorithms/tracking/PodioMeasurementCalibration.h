// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck
// Based on ActsExamples/EventData/MeasurementCalibration.hpp from the ACTS project

#pragma once

#include <Acts/EventData/MultiTrajectory.hpp>
#include <Acts/EventData/SourceLink.hpp>
#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/Utilities/CalibrationContext.hpp>
#include <ActsExamples/EventData/Cluster.hpp>
#include <ActsExamples/EventData/Measurement.hpp>
#include <ActsPlugins/EDM4hep/PodioTrackStateContainer.hpp>

#include <cassert>

namespace eicrecon {

/// Abstract base class for measurement-based calibration compatible with Podio backend
class PodioMeasurementCalibrator {
public:
  virtual void
  calibrate(const ActsExamples::MeasurementContainer& measurements,
            const ActsExamples::ClusterContainer* clusters, const Acts::GeometryContext& gctx,
            const Acts::CalibrationContext& cctx, const Acts::SourceLink& sourceLink,
            ActsPlugins::MutablePodioTrackStateContainer<Acts::RefHolder>::TrackStateProxy&
                trackState) const = 0;

  virtual ~PodioMeasurementCalibrator() = default;
  virtual bool needsClusters() const { return false; }
};

/// Calibrator to convert an index source link to a measurement as-is (Podio version)
class PodioPassThroughCalibrator : public PodioMeasurementCalibrator {
public:
  /// Find the measurement corresponding to the source link.
  ///
  /// @param measurements The measurement container
  /// @param clusters The cluster container (unused)
  /// @param gctx The geometry context (unused)
  /// @param cctx The calibration context (unused)
  /// @param sourceLink The source link to calibrate
  /// @param trackState The track state to calibrate
  void calibrate(const ActsExamples::MeasurementContainer& measurements,
                 const ActsExamples::ClusterContainer* clusters, const Acts::GeometryContext& gctx,
                 const Acts::CalibrationContext& cctx, const Acts::SourceLink& sourceLink,
                 ActsPlugins::MutablePodioTrackStateContainer<Acts::RefHolder>::TrackStateProxy&
                     trackState) const override;
};

/// Adapter class that wraps a PodioMeasurementCalibrator to conform to the
/// core ACTS calibration interface for Podio track state containers
class PodioMeasurementCalibratorAdapter {
public:
  PodioMeasurementCalibratorAdapter(const PodioMeasurementCalibrator& calibrator,
                                    const ActsExamples::MeasurementContainer& measurements,
                                    const ActsExamples::ClusterContainer* clusters = nullptr);

  PodioMeasurementCalibratorAdapter() = delete;

  void calibrate(const Acts::GeometryContext& gctx, const Acts::CalibrationContext& cctx,
                 const Acts::SourceLink& sourceLink,
                 ActsPlugins::MutablePodioTrackStateContainer<Acts::RefHolder>::TrackStateProxy
                     trackState) const;

private:
  const PodioMeasurementCalibrator& m_calibrator;
  const ActsExamples::MeasurementContainer& m_measurements;
  const ActsExamples::ClusterContainer* m_clusters;
};

} // namespace eicrecon
