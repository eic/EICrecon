// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Dmitry Romanov, Shujie Li

#include "CKFTracking.h"

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/EventData/GenericBoundTrackParameters.hpp>
#include <Acts/EventData/Measurement.hpp>
#include <Acts/EventData/MultiTrajectory.hpp>
#include <Acts/EventData/MultiTrajectoryHelpers.hpp>
#include <Acts/EventData/ParticleHypothesis.hpp>
#include <Acts/EventData/SourceLink.hpp>
#include <Acts/EventData/TrackContainer.hpp>
#include <Acts/EventData/TrackProxy.hpp>
#include <Acts/EventData/TrackStateType.hpp>
#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Propagator/Propagator.hpp>
#include <Acts/Surfaces/PerigeeSurface.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/TrackFitting/GainMatrixSmoother.hpp>
#include <Acts/TrackFitting/GainMatrixUpdater.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <ActsExamples/EventData/IndexSourceLink.hpp>
#include <ActsExamples/EventData/Measurement.hpp>
#include <ActsExamples/EventData/MeasurementCalibration.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <edm4eic/Cov2f.h>
#include <edm4eic/Cov3f.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrajectoryCollection.h>
#include <edm4hep/Vector2f.h>
#include <fmt/core.h>
#include <Eigen/Core>
#include <algorithm>
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
                 {m_cfg.m_etaBins, m_cfg.m_chi2CutOff,
                  {m_cfg.m_numMeasurementsCutOff.begin(), m_cfg.m_numMeasurementsCutOff.end()}
                 }
                },
        };
        m_trackFinderFunc = CKFTracking::makeCKFTrackingFunction(m_geoSvc->trackingGeometry(), m_BField, logger());
    }

    std::tuple<
        std::unique_ptr<edm4eic::TrajectoryCollection>,
        std::unique_ptr<edm4eic::TrackParametersCollection>,
        std::vector<ActsExamples::Trajectories*>,
        std::vector<ActsExamples::ConstTrackContainer*>
    >
    CKFTracking::process(const edm4eic::Measurement2DCollection& meas2Ds,
                         const edm4eic::TrackParametersCollection &init_trk_params) {


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

            auto measurement = Acts::makeMeasurement(Acts::SourceLink{sourceLink}, loc, cov, Acts::eBoundLoc0, Acts::eBoundLoc1);
            measurements->emplace_back(std::move(measurement));

            hit_index++;
        }

        ActsExamples::TrackParametersContainer acts_init_trk_params;
        for (const auto& track_parameter: init_trk_params) {

            Acts::BoundVector params;
            params(Acts::eBoundLoc0)   = track_parameter.getLoc().a * Acts::UnitConstants::mm;  // cylinder radius
            params(Acts::eBoundLoc1)   = track_parameter.getLoc().b * Acts::UnitConstants::mm;  // cylinder length
            params(Acts::eBoundTheta)  = track_parameter.getTheta();
            params(Acts::eBoundPhi)    = track_parameter.getPhi();
            params(Acts::eBoundQOverP) = track_parameter.getQOverP() / Acts::UnitConstants::GeV;
            params(Acts::eBoundTime)   = track_parameter.getTime() * Acts::UnitConstants::ns;

            double charge = track_parameter.getCharge();

            Acts::BoundSquareMatrix cov                 = Acts::BoundSquareMatrix::Zero();
            cov(Acts::eBoundLoc0, Acts::eBoundLoc0)     = std::pow( track_parameter.getLocError().xx ,2)*Acts::UnitConstants::mm*Acts::UnitConstants::mm;
            cov(Acts::eBoundLoc1, Acts::eBoundLoc1)     = std::pow( track_parameter.getLocError().yy,2)*Acts::UnitConstants::mm*Acts::UnitConstants::mm;
            cov(Acts::eBoundTheta, Acts::eBoundTheta)   = std::pow( track_parameter.getMomentumError().xx,2);
            cov(Acts::eBoundPhi, Acts::eBoundPhi)       = std::pow( track_parameter.getMomentumError().yy,2);
            cov(Acts::eBoundQOverP, Acts::eBoundQOverP) = std::pow( track_parameter.getMomentumError().zz,2) / (Acts::UnitConstants::GeV*Acts::UnitConstants::GeV);
            cov(Acts::eBoundTime, Acts::eBoundTime)     = std::pow( track_parameter.getTimeError(),2)*Acts::UnitConstants::ns*Acts::UnitConstants::ns;

            // Construct a perigee surface as the target surface
            auto pSurface = Acts::Surface::makeShared<const Acts::PerigeeSurface>(Acts::Vector3(0,0,0));

            // Create parameters
            acts_init_trk_params.emplace_back(pSurface, params, cov, Acts::ParticleHypothesis::pion());
        }

        auto trajectories = std::make_unique<edm4eic::TrajectoryCollection>();
        auto track_parameters = std::make_unique<edm4eic::TrackParametersCollection>();

        //// Construct a perigee surface as the target surface
        auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3{0., 0., 0.});

        ACTS_LOCAL_LOGGER(eicrecon::getSpdlogLogger("CKF", m_log, {"^No tracks found$"}));

        Acts::PropagatorPlainOptions pOptions;
        pOptions.maxSteps = 10000;

        ActsExamples::PassThroughCalibrator pcalibrator;
        ActsExamples::MeasurementCalibratorAdapter calibrator(pcalibrator, *measurements);
        Acts::GainMatrixUpdater kfUpdater;
        Acts::GainMatrixSmoother kfSmoother;
        Acts::MeasurementSelector measSel{m_sourcelinkSelectorCfg};

        Acts::CombinatorialKalmanFilterExtensions<Acts::VectorMultiTrajectory>
                extensions;
        extensions.calibrator.connect<&ActsExamples::MeasurementCalibratorAdapter::calibrate>(
                &calibrator);
        extensions.updater.connect<
                &Acts::GainMatrixUpdater::operator()<Acts::VectorMultiTrajectory>>(
                &kfUpdater);
        extensions.smoother.connect<
                &Acts::GainMatrixSmoother::operator()<Acts::VectorMultiTrajectory>>(
                &kfSmoother);
        extensions.measurementSelector.connect<
                &Acts::MeasurementSelector::select<Acts::VectorMultiTrajectory>>(
                &measSel);

        ActsExamples::IndexSourceLinkAccessor slAccessor;
        slAccessor.container = &src_links;
        Acts::SourceLinkAccessorDelegate<ActsExamples::IndexSourceLinkAccessor::Iterator>
                slAccessorDelegate;
        slAccessorDelegate.connect<&ActsExamples::IndexSourceLinkAccessor::range>(&slAccessor);

        // Set the CombinatorialKalmanFilter options
        CKFTracking::TrackFinderOptions options(
                m_geoctx, m_fieldctx, m_calibctx, slAccessorDelegate,
                extensions, pOptions, &(*pSurface));

        // Create track container
        auto trackContainer = std::make_shared<Acts::VectorTrackContainer>();
        auto trackStateContainer = std::make_shared<Acts::VectorMultiTrajectory>();
        ActsExamples::TrackContainer tracks(trackContainer, trackStateContainer);

        // Add seed number column
        tracks.addColumn<unsigned int>("seed");
        Acts::TrackAccessor<unsigned int> seedNumber("seed");

        // Loop over seeds
        for (std::size_t iseed = 0; iseed < acts_init_trk_params.size(); ++iseed) {
            auto result =
                (*m_trackFinderFunc)(acts_init_trk_params.at(iseed), options, tracks);

            if (!result.ok()) {
                m_log->debug("Track finding failed for seed {} with error {}", iseed, result.error());
                continue;
            }

            // Set seed number for all found tracks
            auto& tracksForSeed = result.value();
            for (auto& track : tracksForSeed) {
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
        const Acts::ConstTrackAccessor<unsigned int> constSeedNumber("seed");


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


        // Loop over trajectories
        for (const auto* traj : acts_trajectories) {
          // The trajectory entry indices and the multiTrajectory
          const auto& trackTips = traj->tips();
          const auto& mj = traj->multiTrajectory();
          if (trackTips.empty()) {
            m_log->warn("Empty multiTrajectory.");
            continue;
          }

          // Loop over all trajectories in a multiTrajectory
          // FIXME: we only retain the first trackTips entry
          for (auto trackTip : decltype(trackTips){trackTips.front()}) {
            // Collect the trajectory summary info
            auto trajectoryState =
                Acts::MultiTrajectoryHelpers::trajectoryState(mj, trackTip);

            // Check if the reco track has fitted track parameters
            if (not traj->hasTrackParameters(trackTip)) {
              m_log->warn(
                  "No fitted track parameters for trajectory with entry index = {}",
                  trackTip);
              continue;
            }

            // Create trajectory
            auto trajectory = trajectories->create();
            trajectory.setChi2(trajectoryState.chi2Sum);
            trajectory.setNdf(trajectoryState.NDF);
            trajectory.setNMeasurements(trajectoryState.nMeasurements);
            trajectory.setNStates(trajectoryState.nStates);
            trajectory.setNOutliers(trajectoryState.nOutliers);
            trajectory.setNHoles(trajectoryState.nHoles);
            trajectory.setNSharedHits(trajectoryState.nSharedHits);

            m_log->debug("trajectory state, measurement, outlier, hole: {} {} {} {}",
                trajectoryState.nStates,
                trajectoryState.nMeasurements,
                trajectoryState.nOutliers,
                trajectoryState.nHoles);

            for (const auto& measurementChi2 : trajectoryState.measurementChi2) {
                trajectory.addToMeasurementChi2(measurementChi2);
            }

            for (const auto& outlierChi2 : trajectoryState.outlierChi2) {
                trajectory.addToOutlierChi2(outlierChi2);
            }

            // Get the fitted track parameter
            const auto& boundParam = traj->trackParameters(trackTip);
            const auto& parameter  = boundParam.parameters();
            const auto& covariance = *boundParam.covariance();

            edm4eic::MutableTrackParameters pars{
                0, // type: track head --> 0
                {
                    static_cast<float>(parameter[Acts::eBoundLoc0]),
                    static_cast<float>(parameter[Acts::eBoundLoc1])
                },
                {
                    static_cast<float>(covariance(Acts::eBoundLoc0, Acts::eBoundLoc0)),
                    static_cast<float>(covariance(Acts::eBoundLoc1, Acts::eBoundLoc1)),
                    static_cast<float>(covariance(Acts::eBoundLoc0, Acts::eBoundLoc1))
                },
                static_cast<float>(parameter[Acts::eBoundTheta]),
                static_cast<float>(parameter[Acts::eBoundPhi]),
                static_cast<float>(parameter[Acts::eBoundQOverP]),
                {
                    static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundTheta)),
                    static_cast<float>(covariance(Acts::eBoundPhi, Acts::eBoundPhi)),
                    static_cast<float>(covariance(Acts::eBoundQOverP, Acts::eBoundQOverP)),
                    static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundPhi)),
                    static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundQOverP)),
                    static_cast<float>(covariance(Acts::eBoundPhi, Acts::eBoundQOverP))
                },
                static_cast<float>(parameter[Acts::eBoundTime]),
                sqrt(static_cast<float>(covariance(Acts::eBoundTime, Acts::eBoundTime))),
                static_cast<float>(boundParam.charge())};

            track_parameters->push_back(pars);
            trajectory.addToTrackParameters(pars);

            // save measurement2d to good measurements or outliers according to srclink index
            // fix me: ideally, this should be integrated into multitrajectoryhelper
            // fix me: should say "OutlierMeasurements" instead of "OutlierHits" etc
            mj.visitBackwards(trackTip, [&](const auto& state) {

                auto geoID = state.referenceSurface().geometryId().value();
                auto typeFlags = state.typeFlags();

                // find the associated hit (2D measurement) with state sourcelink index
                // fix me: calibrated or not?
                if (state.hasUncalibratedSourceLink()) {

                    std::size_t srclink_index = state.getUncalibratedSourceLink().template get<ActsExamples::IndexSourceLink>().index();

                    // no hit on this state/surface, skip
                    if (typeFlags.test(Acts::TrackStateFlag::HoleFlag)) {
                        m_log->debug("No hit found on geo id={}", geoID);

                    } else {
                        auto meas2D = meas2Ds[srclink_index];
                        if (typeFlags.test(Acts::TrackStateFlag::MeasurementFlag)) {
                            trajectory.addToMeasurementHits(meas2D);
                            m_log->debug("Measurement on geo id={}, index={}, loc={},{}",
                                geoID, srclink_index, meas2D.getLoc().a, meas2D.getLoc().b);

                        }
                        else if (typeFlags.test(Acts::TrackStateFlag::OutlierFlag)) {
                            trajectory.addToOutlierHits(meas2D);
                            m_log->debug("Outlier on geo id={}, index={}, loc={},{}",
                                geoID, srclink_index, meas2D.getLoc().a, meas2D.getLoc().b);

                        }
                    }
                }

            });

          }
        }

        return std::make_tuple(std::move(trajectories), std::move(track_parameters), std::move(acts_trajectories), std::move(constTracks_v));
    }

} // namespace eicrecon
