// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck

#pragma once

#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <Acts/MagneticField/MagneticFieldProvider.hpp>
#include <Acts/TrackFinding/CombinatorialKalmanFilter.hpp>
#include <Acts/TrackFinding/MeasurementSelector.hpp>
#include <Acts/Utilities/CalibrationContext.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <Acts/Utilities/Result.hpp>
#if Acts_VERSION_MAJOR < 39
#include <ActsExamples/EventData/IndexSourceLink.hpp>
#endif
#include <ActsExamples/EventData/Track.hpp>
#include <ActsPlugins/EDM4hep/PodioTrackContainer.hpp>
#include <ActsPlugins/EDM4hep/PodioTrackStateContainer.hpp>
#include <ActsPlugins/EDM4hep/PodioUtil.hpp>
#include <ActsPodioEdm/TrackCollection.h>
#include <ActsPodioEdm/TrackStateCollection.h>
#include <algorithms/algorithm.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackParametersCollection.h>
#include <memory>
#include <string_view>

#include "CKFTrackingConfig.h"
#include "PodioGeometryIdConversionHelper.h"
#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/interfaces/ActsSvc.h"

class ActsGeometryProvider;

namespace eicrecon {

using CKFTrackingAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TrackParametersCollection, edm4eic::Measurement2DCollection>,
    algorithms::Output<ActsPodioEdm::TrackStateCollection, ActsPodioEdm::BoundParametersCollection,
                       ActsPodioEdm::JacobianCollection, ActsPodioEdm::TrackCollection>>;

/** Fitting algorithm implementation .
 *
 * \ingroup tracking
 */

class CKFTracking : public CKFTrackingAlgorithm, public WithPodConfig<eicrecon::CKFTrackingConfig> {
public:
  // Use Podio track container type
  using PodioTrackContainer =
      Acts::TrackContainer<ActsPlugins::MutablePodioTrackContainer<Acts::RefHolder>,
                           ActsPlugins::MutablePodioTrackStateContainer<Acts::RefHolder>,
                           std::shared_ptr>;

  /// Track finder function that takes input measurements, initial trackstate
  /// and track finder options and returns some track-finder-specific result.
#if Acts_VERSION_MAJOR >= 39
  using TrackFinderOptions = Acts::CombinatorialKalmanFilterOptions<PodioTrackContainer>;
#else
  using TrackFinderOptions =
      Acts::CombinatorialKalmanFilterOptions<ActsExamples::IndexSourceLinkAccessor::Iterator,
                                             PodioTrackContainer>;
#endif
  using TrackFinderResult = Acts::Result<std::vector<PodioTrackContainer::TrackProxy>>;

  /// Find function that takes the above parameters
  /// @note This is separated into a virtual interface to keep compilation units
  /// small
  class CKFTrackingFunction {
  public:
    virtual ~CKFTrackingFunction() = default;

    virtual TrackFinderResult operator()(const ActsExamples::TrackParameters&,
                                         const TrackFinderOptions&, PodioTrackContainer&) const = 0;
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
  const algorithms::ActsSvc& m_actsSvc{algorithms::ActsSvc::instance()};
  std::shared_ptr<const ActsGeometryProvider> m_geoSvc{m_actsSvc.acts_geometry_provider()};

  Acts::MeasurementSelector::Config m_sourcelinkSelectorCfg;

  /// Private access to the logging instance
  const Acts::Logger& acts_logger() const { return *m_acts_logger; }
};

} // namespace eicrecon
