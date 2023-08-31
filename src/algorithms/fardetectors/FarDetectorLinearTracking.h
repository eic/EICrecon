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

	/** One time initialization **/
	void init(const dd4hep::BitFieldCoder *id_dec, std::shared_ptr<spdlog::logger>& logger);

	/** Event by event processing **/
	std::unique_ptr<edm4eic::TrackParametersCollection> produce(const edm4hep::TrackerHitCollection &inputhits);

    private:
	const dd4hep::BitFieldCoder *m_id_dec{nullptr};
	std::shared_ptr<spdlog::logger> m_log;

    };

} // eicrecon
