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
#include <edm4eic/Cov3f.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrajectoryCollection.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <Eigen/Core>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <functional>
#include <list>
#include <optional>
#include <utility>

#if EDM4EIC_VERSION_MAJOR >= 5
#include <edm4eic/Cov6f.h>
#endif

#include "ActsGeometryProvider.h"
#include "DD4hepBField.h"
#include "extensions/spdlog/SpdlogFormatters.h" // IWYU pragma: keep
#include "extensions/spdlog/SpdlogToActs.h"

namespace eicrecon {

    using namespace Acts::UnitLiterals;

    #if EDM4EIC_VERSION_MAJOR >= 5
      // This array relates the Acts and EDM4eic covariance matrices, including
      // the unit conversion to get from Acts units into EDM4eic units.
      //
      // Note: std::map is not constexpr, so we use a constexpr std::array
      // std::array initialization need double braces since arrays are aggregates
      // ref: https://en.cppreference.com/w/cpp/language/aggregate_initialization
      static constexpr std::array<std::pair<Acts::BoundIndices, double>, 6> edm4eic_indexed_units{{
        {Acts::eBoundLoc0, Acts::UnitConstants::mm},
        {Acts::eBoundLoc1, Acts::UnitConstants::mm},
        {Acts::eBoundTheta, 1.},
        {Acts::eBoundPhi, 1.},
        {Acts::eBoundQOverP, 1. / Acts::UnitConstants::GeV},
        {Acts::eBoundTime, Acts::UnitConstants::ns}
      }};
    #endif

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
        std::unique_ptr<edm4eic::TrackCollection>,
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
            cov(1, 0) = meas2D.getCovariance().xy;

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

            double charge = std::copysign(1., track_parameter.getQOverP());

            Acts::BoundSquareMatrix cov = Acts::BoundSquareMatrix::Zero();
            #if EDM4EIC_VERSION_MAJOR >= 5
              for (size_t i = 0; const auto& [a, x] : edm4eic_indexed_units) {
                for (size_t j = 0; const auto& [b, y] : edm4eic_indexed_units) {
                  cov(a, b) = track_parameter.getCovariance()(i,j) * x * y;
                  ++j;
                }
                ++i;
              }
            #else
              cov(Acts::eBoundLoc0, Acts::eBoundLoc0)     = std::pow( track_parameter.getLocError().xx ,2)*Acts::UnitConstants::mm*Acts::UnitConstants::mm;
              cov(Acts::eBoundLoc1, Acts::eBoundLoc1)     = std::pow( track_parameter.getLocError().yy,2)*Acts::UnitConstants::mm*Acts::UnitConstants::mm;
              cov(Acts::eBoundTheta, Acts::eBoundTheta)   = std::pow( track_parameter.getMomentumError().xx,2);
              cov(Acts::eBoundPhi, Acts::eBoundPhi)       = std::pow( track_parameter.getMomentumError().yy,2);
              cov(Acts::eBoundQOverP, Acts::eBoundQOverP) = std::pow( track_parameter.getMomentumError().zz,2) / (Acts::UnitConstants::GeV*Acts::UnitConstants::GeV);
              cov(Acts::eBoundTime, Acts::eBoundTime)     = std::pow( track_parameter.getTimeError(),2)*Acts::UnitConstants::ns*Acts::UnitConstants::ns;
            #endif

            // Construct a perigee surface as the target surface
            auto pSurface = Acts::Surface::makeShared<const Acts::PerigeeSurface>(Acts::Vector3(0,0,0));

            // Create parameters
            acts_init_trk_params.emplace_back(pSurface, params, cov, Acts::ParticleHypothesis::pion());
        }

        auto trajectories = std::make_unique<edm4eic::TrajectoryCollection>();
        auto track_parameters = std::make_unique<edm4eic::TrackParametersCollection>();
        auto tracks = std::make_unique<edm4eic::TrackCollection>();

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
        ActsExamples::TrackContainer acts_tracks(trackContainer, trackStateContainer);

        // Add seed number column
        acts_tracks.addColumn<unsigned int>("seed");
        Acts::TrackAccessor<unsigned int> seedNumber("seed");

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
            #if EDM4EIC_VERSION_MAJOR < 5
              trajectory.setChi2(trajectoryState.chi2Sum);
              trajectory.setNdf(trajectoryState.NDF);
            #endif
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

            auto pars = track_parameters->create();
            pars.setType(0); // type: track head --> 0
            pars.setLoc({
                  static_cast<float>(parameter[Acts::eBoundLoc0]),
                  static_cast<float>(parameter[Acts::eBoundLoc1])
              });
            pars.setTheta(static_cast<float>(parameter[Acts::eBoundTheta]));
            pars.setPhi(static_cast<float>(parameter[Acts::eBoundPhi]));
            pars.setQOverP(static_cast<float>(parameter[Acts::eBoundQOverP]));
            pars.setTime(static_cast<float>(parameter[Acts::eBoundTime]));
            #if EDM4EIC_VERSION_MAJOR >= 5
              edm4eic::Cov6f cov;
              for (size_t i = 0; const auto& [a, x] : edm4eic_indexed_units) {
                for (size_t j = 0; const auto& [b, y] : edm4eic_indexed_units) {
                  // FIXME why not pars.getCovariance()(i,j) = covariance(a,b) / x / y;
                  cov(i,j) = covariance(a,b) / x / y;
                }
              }
              pars.setCovariance(cov);
            #else
              pars.setCharge(static_cast<float>(boundParam.charge()));
              pars.setLocError({
                    static_cast<float>(covariance(Acts::eBoundLoc0, Acts::eBoundLoc0)),
                    static_cast<float>(covariance(Acts::eBoundLoc1, Acts::eBoundLoc1)),
                    static_cast<float>(covariance(Acts::eBoundLoc0, Acts::eBoundLoc1))
                });
              pars.setMomentumError({
                    static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundTheta)),
                    static_cast<float>(covariance(Acts::eBoundPhi, Acts::eBoundPhi)),
                    static_cast<float>(covariance(Acts::eBoundQOverP, Acts::eBoundQOverP)),
                    static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundPhi)),
                    static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundQOverP)),
                    static_cast<float>(covariance(Acts::eBoundPhi, Acts::eBoundQOverP))
                });
              pars.setTimeError(sqrt(static_cast<float>(covariance(Acts::eBoundTime, Acts::eBoundTime))));
            #endif

            trajectory.addToTrackParameters(pars);

            // Fill tracks
            #if EDM4EIC_VERSION_MAJOR >= 5
              auto track = tracks->create();
              track.setType(                             // Flag that defines the type of track
                pars.getType()
              );
              track.setPosition(                         // Track 3-position at the vertex
                edm4hep::Vector3f()
              );
              track.setMomentum(                         // Track 3-momentum at the vertex [GeV]
                edm4hep::Vector3f()
              );
              track.setPositionMomentumCovariance(       // Covariance matrix in basis [x,y,z,px,py,pz]
                edm4eic::Cov6f()
              );
              track.setTime(                             // Track time at the vertex [ns]
                static_cast<float>(parameter[Acts::eBoundTime])
              );
              track.setTimeError(                        // Error on the track vertex time
                sqrt(static_cast<float>(covariance(Acts::eBoundTime, Acts::eBoundTime)))
              );
              track.setCharge(                           // Particle charge
                std::copysign(1., parameter[Acts::eBoundQOverP])
              );
              track.setChi2(trajectoryState.chi2Sum);    // Total chi2
              track.setNdf(trajectoryState.NDF);         // Number of degrees of freedom
              track.setPdg(                              // PDG particle ID hypothesis
                boundParam.particleHypothesis().absolutePdg()
              );
              track.setTrajectory(trajectory);           // Trajectory of this track
            #endif

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
                          #if EDM4EIC_VERSION_MAJOR >= 5
                            track.addToMeasurements(meas2D);
                            trajectory.addToMeasurements_deprecated(meas2D);
                          #else
                            trajectory.addToMeasurementHits(meas2D);
                          #endif
                          m_log->debug("Measurement on geo id={}, index={}, loc={},{}",
                                geoID, srclink_index, meas2D.getLoc().a, meas2D.getLoc().b);

                        }
                        else if (typeFlags.test(Acts::TrackStateFlag::OutlierFlag)) {
                          #if EDM4EIC_VERSION_MAJOR >= 5
                            trajectory.addToOutliers_deprecated(meas2D);
                          #else
                            trajectory.addToOutlierHits(meas2D);
                          #endif
                          m_log->debug("Outlier on geo id={}, index={}, loc={},{}",
                                geoID, srclink_index, meas2D.getLoc().a, meas2D.getLoc().b);

                        }
                    }
                }

            });

          }
        }

        return std::make_tuple(std::move(trajectories), std::move(track_parameters), std::move(tracks), std::move(acts_trajectories), std::move(constTracks_v));
    }

} // namespace eicrecon
