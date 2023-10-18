// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Dmitry Romanov

#include "CKFTracking.h"

#include <DDRec/CellIDPositionConverter.h>
#include <DDRec/SurfaceManager.h>
#include <DDRec/Surface.h>

#include <Acts/EventData/TrackParameters.hpp>
#include <Acts/EventData/MultiTrajectory.hpp>
#include <Acts/EventData/VectorMultiTrajectory.hpp>

#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/Plugins/DD4hep/DD4hepDetectorElement.hpp>
#include <Acts/Surfaces/PerigeeSurface.hpp>

#include <Acts/TrackFitting/GainMatrixSmoother.hpp>
#include <Acts/TrackFitting/GainMatrixUpdater.hpp>
#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Propagator/Navigator.hpp>
#include <Acts/Propagator/Propagator.hpp>
#include <Acts/Definitions/Common.hpp>
#include <Acts/Utilities/Helpers.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <Acts/Definitions/Units.hpp>

#include "extensions/spdlog/SpdlogToActs.h"
#include "extensions/spdlog/SpdlogFormatters.h"

#include "DD4hepBField.h"

#include "ActsExamples/EventData/GeometryContainers.hpp"
#include "ActsExamples/EventData/Measurement.hpp"
#include "ActsExamples/EventData/Index.hpp"
#include "ActsExamples/EventData/IndexSourceLink.hpp"
#include "ActsExamples/EventData/Track.hpp"

#include "ActsGeometryProvider.h"

#include <edm4eic/TrackerHitCollection.h>
#include <edm4eic/TrajectoryCollection.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/Measurement2DCollection.h>

#include <spdlog/fmt/ostr.h>

namespace eicrecon {

    using namespace Acts::UnitLiterals;

    CKFTracking::CKFTracking() {
    }

    void CKFTracking::init(std::shared_ptr<const ActsGeometryProvider> geo_svc, std::shared_ptr<spdlog::logger> log) {
        m_log = log;

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
        m_trackFinderFunc = CKFTracking::makeCKFTrackingFunction(m_geoSvc->trackingGeometry(), m_BField);
    }

    std::tuple<
        std::unique_ptr<edm4eic::TrajectoryCollection>,
        std::unique_ptr<edm4eic::TrackParametersCollection>,
        std::vector<ActsExamples::Trajectories*>
    >
    CKFTracking::process(const edm4eic::Measurement2DCollection& meas2Ds,
                         const edm4eic::TrackParametersCollection &init_trk_params) {

        // auto src_links    = std::vector<std::shared_ptr<ActsExamples::IndexSourceLink>>();

        // create sourcelink and measurement containers
        auto measurements = std::make_shared<ActsExamples::MeasurementContainer>();
        ActsExamples::IndexSourceLinkContainer src_links;

        std::size_t  hit_index = 0;
        for (const auto& meas2D : meas2Ds) {
            // Create source links
            auto sourceLink = std::make_shared<ActsExamples::IndexSourceLink>(meas2D.getSurface(), hit_index);
            // src_links.emplace_back(sourceLink);
            src_links.emplace_hint(src_links.end(), *sourceLink);

            // Create ACTS measurements
            Acts::Vector2 loc = Acts::Vector2::Zero();
            auto pos_x = meas2D.getLoc().a;
            auto pos_y = meas2D.getLoc().b;
            loc[Acts::eBoundLoc0] = pos_x;
            loc[Acts::eBoundLoc1] = pos_y;


            Acts::SymMatrix2 cov = Acts::SymMatrix2::Zero();
            cov(0, 0) = meas2D.getCovariance().xx;
            cov(1, 1) = meas2D.getCovariance().yy;

            auto measurement = Acts::makeMeasurement(*sourceLink, loc, cov, Acts::eBoundLoc0, Acts::eBoundLoc1);
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

            Acts::BoundSymMatrix cov                    = Acts::BoundSymMatrix::Zero();
            cov(Acts::eBoundLoc0, Acts::eBoundLoc0)     = std::pow( track_parameter.getLocError().xx ,2)*Acts::UnitConstants::mm*Acts::UnitConstants::mm;
            cov(Acts::eBoundLoc1, Acts::eBoundLoc1)     = std::pow( track_parameter.getLocError().yy,2)*Acts::UnitConstants::mm*Acts::UnitConstants::mm;
            cov(Acts::eBoundTheta, Acts::eBoundTheta)   = std::pow( track_parameter.getMomentumError().xx,2);
            cov(Acts::eBoundPhi, Acts::eBoundPhi)       = std::pow( track_parameter.getMomentumError().yy,2);
            cov(Acts::eBoundQOverP, Acts::eBoundQOverP) = std::pow( track_parameter.getMomentumError().zz,2) / (Acts::UnitConstants::GeV*Acts::UnitConstants::GeV);
            cov(Acts::eBoundTime, Acts::eBoundTime)     = std::pow( track_parameter.getTimeError(),2)*Acts::UnitConstants::ns*Acts::UnitConstants::ns;

            // Construct a perigee surface as the target surface
            auto pSurface = Acts::Surface::makeShared<const Acts::PerigeeSurface>(Acts::Vector3(0,0,0));

            // Create parameters
            acts_init_trk_params.emplace_back(pSurface, params, charge, cov);
        }

        auto trajectories = std::make_unique<edm4eic::TrajectoryCollection>();
        auto track_parameters = std::make_unique<edm4eic::TrackParametersCollection>();

        std::vector<ActsExamples::Trajectories*> acts_trajectories;
        acts_trajectories.reserve(init_trk_params.size());

        //// Construct a perigee surface as the target surface
        auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3{0., 0., 0.});

        ACTS_LOCAL_LOGGER(eicrecon::getSpdlogLogger(m_log, {"^No tracks found$"}));

        Acts::PropagatorPlainOptions pOptions;
        pOptions.maxSteps = 10000;

        ActsExamples::MeasurementCalibrator calibrator{*measurements};
        Acts::GainMatrixUpdater kfUpdater;
        Acts::GainMatrixSmoother kfSmoother;
        Acts::MeasurementSelector measSel{m_sourcelinkSelectorCfg};
        //Acts::MeasurementSelector measSel;

        Acts::CombinatorialKalmanFilterExtensions<Acts::VectorMultiTrajectory>
                extensions;
        extensions.calibrator.connect<&ActsExamples::MeasurementCalibrator::calibrate>(&calibrator);
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
                extensions, Acts::LoggerWrapper{logger()}, pOptions, &(*pSurface));

        auto results = (*m_trackFinderFunc)(acts_init_trk_params, options);

        for (std::size_t iseed = 0; iseed < acts_init_trk_params.size(); ++iseed) {

            auto &result = results[iseed];

            if (result.ok()) {

                // Get the track finding output object
                auto &trackFindingOutput = result.value();

                // Create a SimMultiTrajectory
                auto* multiTrajectory = new ActsExamples::Trajectories(
                    std::move(trackFindingOutput.fittedStates),
                    std::move(trackFindingOutput.lastMeasurementIndices),
                    std::move(trackFindingOutput.fittedParameters)
                );

                // Get the entry index for the single trajectory
                // The trajectory entry indices and the multiTrajectory
                const auto& mj        = multiTrajectory->multiTrajectory();
                const auto& trackTips = multiTrajectory->tips();
                // const auto& states    = multiTrajectory->states(); // can't find corresponding function anywhere 


                if (trackTips.empty()) {
                    m_log->debug("Empty multiTrajectory.");
                    delete multiTrajectory;
                    continue;
                }

                const auto& trackTip = trackTips.front();

                // Collect the trajectory summary info
                auto trajectoryState = Acts::MultiTrajectoryHelpers::trajectoryState(mj, trackTip);

                // Create trajectory
                auto trajectory = trajectories->create();

                trajectory.setChi2(trajectoryState.chi2Sum);
                trajectory.setNdf(trajectoryState.NDF);
                trajectory.setNMeasurements(trajectoryState.nMeasurements);
                trajectory.setNStates(trajectoryState.nStates);
                trajectory.setNOutliers(trajectoryState.nOutliers);
                trajectory.setNHoles(trajectoryState.nHoles);
                trajectory.setNSharedHits(trajectoryState.nSharedHits);


                for (const auto& measurementChi2 : trajectoryState.measurementChi2) {
                    trajectory.addToMeasurementChi2(measurementChi2);
                }

                for (const auto& outlierChi2 : trajectoryState.outlierChi2) {
                    trajectory.addToOutlierChi2(outlierChi2);
                }

                // Get the fitted track parameter
                //
                if (multiTrajectory->hasTrackParameters(trackTip)) {

                    const auto& boundParam = multiTrajectory->trackParameters(trackTip);
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
                }

                // save measurement2d to good measurements or outliers according to srclink index
                // fix me: ideally, this should be integrated into multitrajectoryhelper
                // fix me: should say "OutlierMeasurements" instead of "OutlierHits" etc
                mj.visitBackwards(trackTip, [&](const auto& state){
                    auto geoID = state.referenceSurface().geometryId().value();
                    auto typeFlags = state.typeFlags();

                    // no hit on this state/surface, skip
                    if (typeFlags.test(Acts::TrackStateFlag::HoleFlag)) {
                        m_log->debug("No hit found on geo id={}", geoID);
                    }
                    else{
                        // find the associated hit (2D measurement) with sourcelink index
                        std::size_t srclink_index = static_cast<const ActsExamples::IndexSourceLink&>(state.uncalibrated()).index();

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
                });
                acts_trajectories.push_back(std::move(multiTrajectory));
            }
            
         else {

                m_log->debug("Track finding failed for truth seed {} with error: {}", iseed, result.error());

            }

        }

        return std::make_tuple(std::move(trajectories), std::move(track_parameters), std::move(acts_trajectories));
    }

} // namespace eicrecon
