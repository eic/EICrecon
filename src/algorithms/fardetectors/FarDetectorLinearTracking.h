// Created by Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <spdlog/spdlog.h>

// Event Model related classes
#include <edm4eic/TrackParametersCollection.h>

#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include "FarDetectorLinearTrackingConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"


namespace eicrecon {

    class FarDetectorLinearTracking : public WithPodConfig<FarDetectorLinearTrackingConfig>  {

    public:

        FarDetectorLinearTracking() = default; //constructer

	/** One time initialization **/
	void init();

	/** Event by event processing **/
	std::unique_ptr<edm4eic::TrackParametersCollection> produce(const edm4hep::TrackerHitCollection &inputhits);
	
	// Get bit encoder
	dd4hep::BitFieldCoder* getEncoder() {return m_id_dec;}

	// Set bit encoder
	void setEncoder(dd4hep::BitFieldCoder *id_dec) {m_id_dec=id_dec;}


    private:
	eicrecon::FarDetectorLinearTrackingConfig m_cfg;
	dd4hep::BitFieldCoder *m_id_dec{nullptr};


    };

} // eicrecon
