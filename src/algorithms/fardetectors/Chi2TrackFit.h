// Created by Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <spdlog/spdlog.h>

// Event Model related classes
#include <edm4eic/TrackParametersCollection.h>
//#include <edm4eic/TrackPointCollection.h>

#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include "FarTrackerTrackingConfig.h"

namespace eicrecon {

    class Chi2TrackFit {

    public:

        Chi2TrackFit() = default; //constructer

	/** One time initialization **/
	void init();

	/** Event by event processing **/
	std::unique_ptr<edm4eic::TrackParametersCollection> produce(const edm4hep::TrackerHitCollection &inputhits);
	//std::unique_ptr<edm4eic::TrackPointCollection> produce(const edm4hep::TrackerHitCollection &inputhits);

	// Get bit encoder
	dd4hep::BitFieldCoder* getEncoder() {return m_id_dec;}

	// Set bit encoder
	void setEncoder(dd4hep::BitFieldCoder *id_dec) {m_id_dec=id_dec;}

	// Get a configuration to be changed
	eicrecon::FarTrackerTrackingConfig& getConfig() {return m_cfg;}

	// Sets a configuration (config is properly copyible)
	eicrecon::FarTrackerTrackingConfig& applyConfig(eicrecon::FarTrackerTrackingConfig cfg) { m_cfg = cfg; return m_cfg;}


    private:
	eicrecon::FarTrackerTrackingConfig m_cfg;
	dd4hep::BitFieldCoder *m_id_dec{nullptr};


    };

} // eicrecon
