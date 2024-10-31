// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Dmitry Romanov, Shujie Li

#include "CKFTracking.h"

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/EventData/GenericBoundTrackParameters.hpp>
#if Acts_VERSION_MAJOR < 36
#include <Acts/EventData/Measurement.hpp>
#endif
#include <Acts/EventData/MultiTrajectory.hpp>
#include <Acts/EventData/ParticleHypothesis.hpp>
#if Acts_VERSION_MAJOR >= 32
#include "Acts/EventData/ProxyAccessor.hpp"
#endif
#include <Acts/EventData/SourceLink.hpp>
#include <Acts/EventData/TrackContainer.hpp>
#include <Acts/EventData/TrackProxy.hpp>
#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#include <Acts/Geometry/GeometryIdentifier.hpp>
#if Acts_VERSION_MAJOR >= 34
#include "Acts/Propagator/AbortList.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/MaterialInteractor.hpp"
#include "Acts/Propagator/Navigator.hpp"
#endif
#include <Acts/Propagator/Propagator.hpp>
#if Acts_VERSION_MAJOR >= 34
#include "Acts/Propagator/StandardAborters.hpp"
#endif
#include <Acts/Surfaces/PerigeeSurface.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/TrackFitting/GainMatrixSmoother.hpp>
#include <Acts/TrackFitting/GainMatrixUpdater.hpp>
#include <Acts/Utilities/Logger.hpp>
#if Acts_VERSION_MAJOR >= 34
#include "Acts/Utilities/TrackHelpers.hpp"
#endif
#include <ActsExamples/EventData/IndexSourceLink.hpp>
#include <ActsExamples/EventData/Measurement.hpp>
#include <ActsExamples/EventData/MeasurementCalibration.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <edm4eic/Cov3f.h>
#include <edm4eic/Cov6f.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4hep/Vector2f.h>
#include <fmt/core.h>
#include <Eigen/Core>
#include <array>
#include <cmath>
#include <cstddef>
#include <functional>
#include <list>
#include <optional>
#include <utility>

#include "ActsGeometryProvider.h"
#include "DD4hepBField.h"
#include "extensions/spdlog/SpdlogFormatters.h" // IWYU pragma: keep
#include "extensions/spdlog/SpdlogToActs.h"

namespace eicrecon {

    using namespace Acts::UnitLiterals;

    // This array relates the Acts and EDM4eic covariance matrices, including
    // the unit conversion to get from Acts units into EDM4eic units.
    //
    // Note: std::map is not constexpr, so we use a constexpr std::array
    // std::array initialization need double braces since arrays are aggregates
    // ref: https://en.cppreference.com/w/cpp/language/aggregate_initialization
    static constexpr std::array<std::pair<Acts::BoundIndices, double>, 6> edm4eic_indexed_units{{
      {Acts::eBoundLoc0, Acts::UnitConstants::mm},
      {Acts::eBoundLoc1, Acts::UnitConstants::mm},
      {Acts::eBoundPhi, 1.},
      {Acts::eBoundTheta, 1.},
      {Acts::eBoundQOverP, 1. / Acts::UnitConstants::GeV},
      {Acts::eBoundTime, Acts::UnitConstants::ns}
    }};

    CKFTracking::CKFTracking() {
    }

    void CKFTracking::init(std::shared_ptr<const ActsGeometryProvider> geo_svc, std::shared_ptr<spdlog::logger> log) {
        m_log = log;
        m_acts_logger = eicrecon::getSpdlogLogger("CKF", m_log);

        m_geoSvc = geo_svc;

        m_BField = std::dynamic_pointer_cast<const eicrecon::BField::DD4hepBField>(m_geoSvc->getFieldProvider());
        m_fieldctx = eicrecon::BField::BFieldVariant(m_BField);

        // eta bins, chi2 and #sourclinks per surface cutoffs
        m_sourcelinkSelectorCfg = {
                {Acts::GeometryIdentifier(),
                 {m_cfg.etaBins, m_cfg.chi2CutOff,
                  {m_cfg.numMeasurementsCutOff.begin(), m_cfg.numMeasurementsCutOff.end()}
                 }
                },
        };
        m_trackFinderFunc = CKFTracking::makeCKFTrackingFunction(m_geoSvc->trackingGeometry(), m_BField, logger());
    }

    std::tuple<
        std::vector<ActsExamples::Trajectories*>,
        std::vector<ActsExamples::ConstTrackContainer*>
    >
    CKFTracking::process(const edm4eic::TrackParametersCollection& init_trk_params,
                         const edm4eic::Measurement2DCollection& meas2Ds) {


        // create sourcelink and measurement containers
        auto measurements = std::make_shared<ActsExamples::MeasurementContainer>();

          // need list here for stable addresses
        std::list<ActsExamples::IndexSourceLink> sourceLinkStorage;
        ActsExamples::IndexSourceLinkContainer src_links;
        src_links.reserve(meas2Ds.size());
        std::size_t  hit_index = 0;


        for (const auto& meas2D : meas2Ds) {

            // --follow example from ACTS to create source links
            sourceLinkStorage.emplace_back(meas2D.getSurface(), hit_index);
            ActsExamples::IndexSourceLink& sourceLink = sourceLinkStorage.back();
            // Add to output containers:
            // index map and source link container are geometry-ordered.
            // since the input is also geometry-ordered, new items can
            // be added at the end.
            src_links.insert(src_links.end(), sourceLink);
            // ---
            // Create ACTS measurements
            Acts::Vector2 loc = Acts::Vector2::Zero();
            loc[Acts::eBoundLoc0] = meas2D.getLoc().a;
            loc[Acts::eBoundLoc1] = meas2D.getLoc().b;


            Acts::SquareMatrix2 cov = Acts::SquareMatrix2::Zero();
            cov(0, 0) = meas2D.getCovariance().xx;
            cov(1, 1) = meas2D.getCovariance().yy;
            cov(0, 1) = meas2D.getCovariance().xy;
            cov(1, 0) = meas2D.getCovariance().xy;

#if Acts_VERSION_MAJOR >= 36
            auto measurement = ActsExamples::makeFixedSizeMeasurement(
              Acts::SourceLink{sourceLink}, loc, cov, Acts::eBoundLoc0, Acts::eBoundLoc1);
#else
            auto measurement = Acts::makeMeasurement(
              Acts::SourceLink{sourceLink}, loc, cov, Acts::eBoundLoc0, Acts::eBoundLoc1);
#endif
            measurements->emplace_back(std::move(measurement));

            hit_index++;
        }

        ActsExamples::TrackParametersContainer acts_init_trk_params;
        for (const auto& track_parameter: init_trk_params) {

            Acts::BoundVector params;
            params(Acts::eBoundLoc0)   = track_parameter.getLoc().a * Acts::UnitConstants::mm;  // cylinder radius
            params(Acts::eBoundLoc1)   = track_parameter.getLoc().b * Acts::UnitConstants::mm;  // cylinder length
            params(Acts::eBoundPhi)    = track_parameter.getPhi();
            params(Acts::eBoundTheta)  = track_parameter.getTheta();
            params(Acts::eBoundQOverP) = track_parameter.getQOverP() / Acts::UnitConstants::GeV;
            params(Acts::eBoundTime)   = track_parameter.getTime() * Acts::UnitConstants::ns;

            double charge = std::copysign(1., track_parameter.getQOverP());

            Acts::BoundSquareMatrix cov = Acts::BoundSquareMatrix::Zero();
            for (size_t i = 0; const auto& [a, x] : edm4eic_indexed_units) {
              for (size_t j = 0; const auto& [b, y] : edm4eic_indexed_units) {
                cov(a, b) = track_parameter.getCovariance()(i,j) * x * y;
                ++j;
              }
              ++i;
            }

            // Construct a perigee surface as the target surface
            auto pSurface = Acts::Surface::makeShared<const Acts::PerigeeSurface>(Acts::Vector3(0,0,0));

            // Create parameters
            acts_init_trk_params.emplace_back(pSurface, params, cov, Acts::ParticleHypothesis::pion());
        }

        //// Construct a perigee surface as the target surface
        auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3{0., 0., 0.});

        ACTS_LOCAL_LOGGER(eicrecon::getSpdlogLogger("CKF", m_log, {"^No tracks found$"}));

        Acts::PropagatorPlainOptions pOptions;
        pOptions.maxSteps = 10000;

        ActsExamples::PassThroughCalibrator pcalibrator;
        ActsExamples::MeasurementCalibratorAdapter calibrator(pcalibrator, *measurements);
        Acts::GainMatrixUpdater kfUpdater;
#if Acts_VERSION_MAJOR < 34
        Acts::GainMatrixSmoother kfSmoother;
#endif
        Acts::MeasurementSelector measSel{m_sourcelinkSelectorCfg};

        Acts::CombinatorialKalmanFilterExtensions<Acts::VectorMultiTrajectory>
                extensions;
        extensions.calibrator.connect<&ActsExamples::MeasurementCalibratorAdapter::calibrate>(
                &calibrator);
        extensions.updater.connect<
                &Acts::GainMatrixUpdater::operator()<Acts::VectorMultiTrajectory>>(
                &kfUpdater);
#if Acts_VERSION_MAJOR < 34
        extensions.smoother.connect<
                &Acts::GainMatrixSmoother::operator()<Acts::VectorMultiTrajectory>>(
                &kfSmoother);
#endif
        extensions.measurementSelector.connect<
                &Acts::MeasurementSelector::select<Acts::VectorMultiTrajectory>>(
                &measSel);

        ActsExamples::IndexSourceLinkAccessor slAccessor;
        slAccessor.container = &src_links;
        Acts::SourceLinkAccessorDelegate<ActsExamples::IndexSourceLinkAccessor::Iterator>
                slAccessorDelegate;
        slAccessorDelegate.connect<&ActsExamples::IndexSourceLinkAccessor::range>(&slAccessor);

        // Set the CombinatorialKalmanFilter options
#if Acts_VERSION_MAJOR < 34
        CKFTracking::TrackFinderOptions options(
                m_geoctx, m_fieldctx, m_calibctx, slAccessorDelegate,
                extensions, pOptions, &(*pSurface));
#else
        CKFTracking::TrackFinderOptions options(
                m_geoctx, m_fieldctx, m_calibctx, slAccessorDelegate,
                extensions, pOptions);
#endif

#if Acts_VERSION_MAJOR >= 34
        Acts::Propagator<Acts::EigenStepper<>, Acts::Navigator> extrapolator(
            Acts::EigenStepper<>(m_BField),
            Acts::Navigator({m_geoSvc->trackingGeometry()},
                            logger().cloneWithSuffix("Navigator")),
            logger().cloneWithSuffix("Propagator"));

        Acts::PropagatorOptions<Acts::ActionList<Acts::MaterialInteractor>,
                                Acts::AbortList<Acts::EndOfWorldReached>>
            extrapolationOptions(m_geoctx, m_fieldctx);
#endif

        // Create track container
        auto trackContainer = std::make_shared<Acts::VectorTrackContainer>();
        auto trackStateContainer = std::make_shared<Acts::VectorMultiTrajectory>();
        ActsExamples::TrackContainer acts_tracks(trackContainer, trackStateContainer);

        // Add seed number column
        acts_tracks.addColumn<unsigned int>("seed");
#if Acts_VERSION_MAJOR >= 32
        Acts::ProxyAccessor<unsigned int> seedNumber("seed");
#else
        Acts::TrackAccessor<unsigned int> seedNumber("seed");
#endif

        // Loop over seeds
        for (std::size_t iseed = 0; iseed < acts_init_trk_params.size(); ++iseed) {
            auto result =
                (*m_trackFinderFunc)(acts_init_trk_params.at(iseed), options, acts_tracks);

            if (!result.ok()) {
                m_log->debug("Track finding failed for seed {} with error {}", iseed, result.error());
                continue;
            }

            // Set seed number for all found tracks
            auto& tracksForSeed = result.value();
            for (auto& track : tracksForSeed) {

#if Acts_VERSION_MAJOR >=34
                auto smoothingResult = Acts::smoothTrack(m_geoctx, track, logger());
                if (!smoothingResult.ok()) {
                    ACTS_ERROR("Smoothing for seed "
                        << iseed << " and track " << track.index()
                        << " failed with error " << smoothingResult.error());
                    continue;
                }

                auto extrapolationResult = Acts::extrapolateTrackToReferenceSurface(
                    track, *pSurface, extrapolator, extrapolationOptions,
                    Acts::TrackExtrapolationStrategy::firstOrLast, logger());
                if (!extrapolationResult.ok()) {
                    ACTS_ERROR("Extrapolation for seed "
                        << iseed << " and track " << track.index()
                        << " failed with error " << extrapolationResult.error());
                    continue;
                }
#endif

                seedNumber(track) = iseed;
            }
        }


        // Move track states and track container to const containers
        // NOTE Using the non-const containers leads to references to
        // implicitly converted temporaries inside the Trajectories.
        auto constTrackStateContainer =
            std::make_shared<Acts::ConstVectorMultiTrajectory>(
                std::move(*trackStateContainer));

        auto constTrackContainer =
            std::make_shared<Acts::ConstVectorTrackContainer>(
                std::move(*trackContainer));

        // FIXME JANA2 std::vector<T*> requires wrapping ConstTrackContainer, instead of:
        //ConstTrackContainer constTracks(constTrackContainer, constTrackStateContainer);
        std::vector<ActsExamples::ConstTrackContainer*> constTracks_v;
        constTracks_v.push_back(
          new ActsExamples::ConstTrackContainer(
            constTrackContainer,
            constTrackStateContainer));
        auto& constTracks = *(constTracks_v.front());

        // Seed number column accessor
#if Acts_VERSION_MAJOR >= 32
        const Acts::ConstProxyAccessor<unsigned int> constSeedNumber("seed");
#else
        const Acts::ConstTrackAccessor<unsigned int> constSeedNumber("seed");
#endif

        // Prepare the output data with MultiTrajectory, per seed
        std::vector<ActsExamples::Trajectories*> acts_trajectories;
        acts_trajectories.reserve(init_trk_params.size());

        ActsExamples::Trajectories::IndexedParameters parameters;
        std::vector<Acts::MultiTrajectoryTraits::IndexType> tips;

        std::optional<unsigned int> lastSeed;
        for (const auto& track : constTracks) {
          if (!lastSeed) {
            lastSeed = constSeedNumber(track);
          }

          if (constSeedNumber(track) != lastSeed.value()) {
            // make copies and clear vectors
            acts_trajectories.push_back(new ActsExamples::Trajectories(
              constTracks.trackStateContainer(),
              tips, parameters));

            tips.clear();
            parameters.clear();
          }

          lastSeed = constSeedNumber(track);

          tips.push_back(track.tipIndex());
          parameters.emplace(
              std::pair{track.tipIndex(),
                        ActsExamples::TrackParameters{track.referenceSurface().getSharedPtr(),
                                                      track.parameters(), track.covariance(),
                                                      track.particleHypothesis()}});
        }

        if (tips.empty()) {
          m_log->info("Last trajectory is empty");
        }

        // last entry: move vectors
        acts_trajectories.push_back(new ActsExamples::Trajectories(
          constTracks.trackStateContainer(),
          std::move(tips), std::move(parameters)));

        return std::make_tuple(std::move(acts_trajectories), std::move(constTracks_v));
    }

} // namespace eicrecon
