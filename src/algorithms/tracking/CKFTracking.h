// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck

#pragma once

#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/MagneticField/MagneticFieldProvider.hpp>
#include <Acts/TrackFinding/CombinatorialKalmanFilter.hpp>
#include <Acts/TrackFinding/MeasurementSelector.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <Acts/Utilities/Result.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <algorithms/algorithm.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackSeedCollection.h>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "algorithms/interfaces/ActsSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/tracking/ActsDD4hepDetector.h"
#include "algorithms/tracking/CKFTrackingConfig.h"

namespace eicrecon {

using CKFTrackingAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TrackSeedCollection, edm4eic::Measurement2DCollection>,
    algorithms::Output<Acts::ConstVectorMultiTrajectory*, Acts::ConstVectorTrackContainer*>>;

/** Fitting algorithm implementation .
 *
 * \ingroup tracking
 */

class CKFTracking : public CKFTrackingAlgorithm, public WithPodConfig<eicrecon::CKFTrackingConfig> {
public:
  /// Track finder function that takes input measurements, initial trackstate
  /// and track finder options and returns some track-finder-specific result.
  using TrackFinderOptions = Acts::CombinatorialKalmanFilterOptions<ActsExamples::TrackContainer>;
  using TrackFinderResult  = Acts::Result<std::vector<ActsExamples::TrackContainer::TrackProxy>>;

  /// Find function that takes the above parameters
  /// @note This is separated into a virtual interface to keep compilation units
  /// small
  class CKFTrackingFunction {
  public:
    virtual ~CKFTrackingFunction() = default;

    virtual TrackFinderResult operator()(const ActsExamples::TrackParameters&,
                                         const TrackFinderOptions&,
                                         ActsExamples::TrackContainer&) const = 0;
  };

  /// Create the track finder function implementation.
  /// The magnetic field is intentionally given by-value since the variantresults
  /// contains shared_ptr anyways.
  static std::shared_ptr<CKFTrackingFunction>
  makeCKFTrackingFunction(std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry,
                          std::shared_ptr<const Acts::MagneticFieldProvider> magneticField,
                          const Acts::Logger& logger);

  CKFTracking(std::string_view name)
      : CKFTrackingAlgorithm{name,
                             {"inputTrackParameters", "inputMeasurements"},
                             {"outputActsTrackStates", "outputActsTracks"},
                             "Combinatorial Kalman Filter track finding"} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  std::shared_ptr<const Acts::Logger> m_acts_logger{nullptr};
  std::shared_ptr<CKFTrackingFunction> m_trackFinderFunc;
  std::shared_ptr<const eicrecon::ActsDD4hepDetector> m_acts_detector{
      algorithms::ActsSvc::instance().detector()};
  std::shared_ptr<const Acts::MagneticFieldProvider> m_BField{m_acts_detector->field()};

  Acts::MeasurementSelector::Config m_sourcelinkSelectorCfg;

  /// Private access to the logging instance
  const Acts::Logger& acts_logger() const { return *m_acts_logger; }
};

} // namespace eicrecon
