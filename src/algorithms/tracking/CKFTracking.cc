// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Dmitry Romanov

#include "CKFTracking.h"

#include <DDRec/CellIDPositionConverter.h>
#include <DDRec/SurfaceManager.h>
#include <DDRec/Surface.h>

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

#include <spdlog/fmt/ostr.h>

#include <functional>
#include <stdexcept>
#include <vector>
#include <random>
#include <stdexcept>


namespace eicrecon {

    using namespace Acts::UnitLiterals;



    CKFTracking::CKFTracking() {
    }

    void CKFTracking::init(std::shared_ptr<const ActsGeometryProvider> geo_svc, std::shared_ptr<spdlog::logger> log) {
        m_log = log;

        m_geoSvc = geo_svc;

        m_BField = std::static_pointer_cast<const eicrecon::BField::DD4hepBField>(m_geoSvc->getFieldProvider());
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

    std::vector<ActsExamples::Trajectories*> CKFTracking::process(const ActsExamples::IndexSourceLinkContainer &src_links,
                                                                  const ActsExamples::MeasurementContainer &measurements,
                                                                  const edm4eic::TrackParametersCollection &init_trk_params) {

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

        //// Prepare the output data with MultiTrajectory
        // TrajectoryContainer trajectories;

        std::vector<ActsExamples::Trajectories *>trajectories;
        trajectories.reserve(init_trk_params.size());

        //// Construct a perigee surface as the target surface
        auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3{0., 0., 0.});

        auto logLevel = eicrecon::SpdlogToActsLevel(m_geoSvc->getActsRelatedLogger()->level());

        ACTS_LOCAL_LOGGER(Acts::getDefaultLogger("CKFTracking Logger", logLevel));

        Acts::PropagatorPlainOptions pOptions;
        pOptions.maxSteps = 10000;

        ActsExamples::MeasurementCalibrator calibrator{measurements};
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
                trajectories.push_back(new ActsExamples::Trajectories(std::move(trackFindingOutput.fittedStates),
                                                                      std::move(trackFindingOutput.lastMeasurementIndices),
                                                                      std::move(trackFindingOutput.fittedParameters)));
            } else {
                m_log->debug("Track finding failed for truth seed {} with error: {}", iseed, result.error());
            }

        }
        return trajectories;
    }
} // namespace eicrecon
