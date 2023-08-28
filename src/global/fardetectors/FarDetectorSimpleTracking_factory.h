// Created by Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <spdlog/spdlog.h>

// Event Model related classes
#include <edm4eic/TrackParametersCollection.h>
#include <algorithms/fardetectors/Chi2TrackFit.h>
#include <algorithms/fardetectors/FarTrackerTrackingConfig.h>

#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>

namespace eicrecon {

    class FarDetectorSimpleTracking_factory : 
    public JChainFactoryT<edm4eic::TrackParameters, FarTrackerTrackingConfig>{

    public:

      explicit FarDetectorSimpleTracking_factory( std::vector<std::string> default_input_tags, FarTrackerTrackingConfig cfg):
	JChainFactoryT(std::move(default_input_tags),cfg ) {
      }

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:
	std::shared_ptr<spdlog::logger> m_log;              /// Logger for this factory
	eicrecon::Chi2TrackFit          m_reco_algo;        // Actual digitisation algorithm

	dd4hep::BitFieldCoder *id_dec{nullptr};
	std::shared_ptr<JDD4hep_service> m_geoSvc;

    };

} // eicrecon
