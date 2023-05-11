// Created by Alex Jentsch
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <spdlog/spdlog.h>

// Event Model related classes

#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>

namespace eicrecon {

    class LowQ2Tracking_factory : public JChainFactoryT<edm4eic::TrackParameters, NoConfig>{

    public:

        LowQ2Tracking_factory( std::vector<std::string> default_input_tags):
                JChainFactoryT<edm4eic::TrackParameters, NoConfig>(std::move(default_input_tags) ) {
        }
    
        LowQ2Tracking_factory(); //constructer

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

	//----- Define constants here ------


	private:
		std::shared_ptr<spdlog::logger> m_log;              /// Logger for this factory
		std::string m_input_tag  = "TaggerTrackerClusterPositions";
		std::string m_output_tag = "LowQ2Tracks";

    };

} // eicrecon
