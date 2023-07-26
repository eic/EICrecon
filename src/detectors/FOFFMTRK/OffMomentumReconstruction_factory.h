// Created by Alex Jentsch
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <algorithm>
#include <cmath>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "DD4hep/Detector.h"
#include <DDRec/CellIDPositionConverter.h>
#include <DDRec/Surface.h>
#include <DDRec/SurfaceManager.h>
#include "services/geometry/dd4hep/JDD4hep_service.h"

// Event Model related classes
#include <edm4eic/MutableReconstructedParticle.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackerHit.h>
#include <edm4eic/vector_utils.h>
#include <edm4hep/SimTrackerHit.h>

#include "extensions/jana/JChainFactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"
#include <spdlog/logger.h>

namespace eicrecon {

    class OffMomentumReconstruction_factory : public eicrecon::JFactoryPodioT<edm4eic::ReconstructedParticle>{

    public:

	OffMomentumReconstruction_factory(); //constructer

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

	//----- Define constants here ------

	const double local_x_offset = -11.9872; // in mm --> this is from mis-alignment of the detector
	const double local_y_offset = -0.0146;  //in mm --> this is from mis-alignment of the detector
	const double local_x_slope_offset = -14.75315; //in mrad
	const double local_y_slope_offset = -0.0073; //in mrad
	const double crossingAngle = -0.025; //in mrad
	const double nomMomentum = 137.5; //in GEV --> exactly half of the top energy momentum (for proton spectators from deuteron breakup)

	std::string m_readout;
	std::string m_geoSvcName;

	std::shared_ptr<JDD4hep_service> m_geoSvc;
	std::string m_localDetElement;
	std::vector<std::string> u_localDetFields;

	dd4hep::DetElement local;
	size_t local_mask = ~0;
	dd4hep::Detector *detector = nullptr;

	const double aXOMD[2][2] = {{1.6248, 12.966293},
	                           {0.1832, -2.8636535}};
	//const double aXOMD[2][2] = {{-1.3656, 2.9971},
	//                           {-0.14452284, -3.33502319}};
	const double aYOMD[2][2] = {{0.0001674, -28.6003},
	                           {0.0000837, -2.87985}};

	double aXOMDinv[2][2] = {{0.0, 0.0},
	                        {0.0, 0.0}};
	double aYOMDinv[2][2] = {{0.0, 0.0},
	                        {0.0, 0.0}};



	private:
		std::shared_ptr<spdlog::logger> m_log;              /// Logger for this factory
		std::string m_input_tag  = "ForwardOffMTrackerHits";
		std::string m_output_tag = "ForwardOffMRecParticles";

    };

} // eicrecon
