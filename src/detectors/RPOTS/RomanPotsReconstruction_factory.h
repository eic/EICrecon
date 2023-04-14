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
#include <services/geometry/dd4hep/JDD4hep_service.h>

// Event Model related classes
#include <edm4eic/MutableReconstructedParticle.h>
#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/TrackerHit.h>
#include <edm4eic/vector_utils.h>
#include <edm4hep/SimTrackerHit.h>

#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>

namespace eicrecon {

    class RomanPotsReconstruction_factory : public eicrecon::JFactoryPodioT<edm4eic::ReconstructedParticle>{

    public:

	RomanPotsReconstruction_factory(); //constructer

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

	//----- Define constants here ------

	//const double local_x_offset_station_1 = -833.3878326;
	//const double local_x_offset_station_2 = -924.342804;
	const double local_x_slope_offset = -0.00622147;
	const double local_y_slope_offset = -0.0451035;
	const double crossingAngle = -0.025;
	const double nomMomentum = 275.0;

	std::string m_readout;
	std::string m_layerField;
	std::string m_sectorField;
	std::string m_geoSvcName;

	dd4hep::BitFieldCoder *id_dec = nullptr;
	size_t sector_idx{0}, layer_idx{0};

	std::shared_ptr<JDD4hep_service> m_geoSvc;
	std::string m_localDetElement;
	std::vector<std::string> u_localDetFields;

	dd4hep::DetElement local;
	size_t local_mask = ~0;
	dd4hep::Detector *detector = nullptr;

	const double aXRP[2][2] = {{2.102403743, 29.11067626},
	                           {0.186640381, 0.192604619}};
	const double aYRP[2][2] = {{0.0000159900, 3.94082098},
	                           {0.0000079946, -0.1402995}};

	double aXRPinv[2][2] = {{0.0, 0.0},
	                        {0.0, 0.0}};
	double aYRPinv[2][2] = {{0.0, 0.0},
	                        {0.0, 0.0}};



	private:
		std::shared_ptr<spdlog::logger> m_log;              /// Logger for this factory
		std::string m_input_tag  = "ForwardRomanPotHits";
		std::string m_output_tag = "ForwardRomanPotRecParticles";

    };

} // eicrecon
