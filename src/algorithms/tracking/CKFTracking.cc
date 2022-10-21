// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Dmitry Romanov

#include "CKFTracking.h"

#include "DDRec/CellIDPositionConverter.h"
#include "DDRec/SurfaceManager.h"
#include "DDRec/Surface.h"

#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Plugins/DD4hep/DD4hepDetectorElement.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"

#include "Acts/TrackFitting/GainMatrixSmoother.hpp"
#include "Acts/TrackFitting/GainMatrixUpdater.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Definitions/Common.hpp"
#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Definitions/Units.hpp"

//#include "JugBase/DataHandle.h"
#include "JugBase/BField/DD4hepBField.h"

#include "JugTrack/GeometryContainers.hpp"
#include "JugTrack/Measurement.hpp"
#include "JugTrack/Index.hpp"
#include "JugTrack/IndexSourceLink.hpp"
#include "JugTrack/Track.hpp"

#include "ActsGeometryProvider.h"

#include "edm4eic/TrackerHitCollection.h"

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

        m_BField = std::dynamic_pointer_cast<const Jug::BField::DD4hepBField>(m_geoSvc->getFieldProvider());
        m_fieldctx = Jug::BField::BFieldVariant(m_BField);

        // eta bins, chi2 and #sourclinks per surface cutoffs
        m_sourcelinkSelectorCfg = {
                {Acts::GeometryIdentifier(),
                 {m_cfg.m_etaBins, m_cfg.m_chi2CutOff,
                  {m_cfg.m_numMeasurementsCutOff.begin(), m_cfg.m_numMeasurementsCutOff.end()}
                 }
                },
        };
        m_trackFinderFunc = CKFTracking::makeCKFTrackingFunction(m_geoSvc->trackingGeometry(), m_BField);
//    auto im = s_msgMap.find(msgLevel());
//    if (im != s_msgMap.end()) {
//        m_actsLoggingLevel = im->second;
//    }
//    return StatusCode::SUCCESS;
//
//
    }

    std::vector<Jug::Trajectories*> CKFTracking::process(const Jug::IndexSourceLinkContainer &src_links,
                                                         const Jug::MeasurementContainer &measurements,
                                                         const Jug::TrackParametersContainer &init_trk_params) {

        //// Prepare the output data with MultiTrajectory
        // TrajectoryContainer trajectories;

        std::vector<Jug::Trajectories *>trajectories;
        trajectories.reserve(init_trk_params.size());

        //// Construct a perigee surface as the target surface
        auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3{0., 0., 0.});

        ACTS_LOCAL_LOGGER(Acts::getDefaultLogger("CKFTracking Logger", m_actsLoggingLevel));

        Acts::PropagatorPlainOptions pOptions;
        pOptions.maxSteps = 10000;

        Jug::MeasurementCalibrator calibrator{measurements};
        Acts::GainMatrixUpdater kfUpdater;
        Acts::GainMatrixSmoother kfSmoother;
        Acts::MeasurementSelector measSel{m_sourcelinkSelectorCfg};
        //Acts::MeasurementSelector measSel;

        Acts::CombinatorialKalmanFilterExtensions extensions;
        extensions.calibrator.connect<&Jug::MeasurementCalibrator::calibrate>(&calibrator);
        extensions.updater.connect<&Acts::GainMatrixUpdater::operator()>(&kfUpdater);
        extensions.smoother.connect<&Acts::GainMatrixSmoother::operator()>(&kfSmoother);
        extensions.measurementSelector.connect<&Acts::MeasurementSelector::select>(&measSel);

        Jug::IndexSourceLinkAccessor slAccessor;
        slAccessor.container = &src_links;
        Acts::SourceLinkAccessorDelegate<Jug::IndexSourceLinkAccessor::Iterator>
                slAccessorDelegate;
        slAccessorDelegate.connect<&Jug::IndexSourceLinkAccessor::range>(&slAccessor);

        // Set the CombinatorialKalmanFilter options
        CKFTracking::TrackFinderOptions options(
                m_geoctx, m_fieldctx, m_calibctx, slAccessorDelegate,
                extensions, Acts::LoggerWrapper{logger()}, pOptions, &(*pSurface));

        // TODO remove this hack...

//        Jug::TrackParametersContainer init_trk_params_refs;
//        init_trk_params_refs.reserve(init_trk_params.size());
//
//        for(auto init_track: init_trk_params) {
//            init_trk_params_refs.push_back(*init_track);
//        }

        auto results = (*m_trackFinderFunc)(init_trk_params, options);

        for (std::size_t iseed = 0; iseed < init_trk_params.size(); ++iseed) {

            auto &result = results[iseed];

            if (result.ok()) {
                // Get the track finding output object
                const auto &trackFindingOutput = result.value();
                // Create a SimMultiTrajectory
                trajectories.push_back(new Jug::Trajectories(std::move(trackFindingOutput.fittedStates),
                                                           std::move(trackFindingOutput.lastMeasurementIndices),
                                                           std::move(trackFindingOutput.fittedParameters)));
            } else {
                m_log->debug("Track finding failed for truth seed {} with error: {}", iseed, result.error());
            }

        }
        return trajectories;
    }
} // namespace eicrecon
