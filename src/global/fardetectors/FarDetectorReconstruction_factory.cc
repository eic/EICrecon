// Created by Alex Jentsch to do Roman Pots reconstruction
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include "FarDetectorReconstruction_factory.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "extensions/string/StringHelpers.h"

namespace eicrecon {

    void FarDetectorReconstruction_factory::Init() {

	std::string plugin_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");

	auto app = GetApplication();

	//m_log = app->GetService<Log_service>()->logger(m_output_tag);

	m_readout = GetDefaultInputTags()[0];

	m_geoSvc = app->GetService<JDD4hep_service>();


	//if(m_readout.empty()){ m_log->error("READOUT IS EMPTY!"); return; }

    }


    void FarDetectorReconstruction_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    	// Nothing to do here
    }

    void FarDetectorReconstruction_factory::Process(const std::shared_ptr<const JEvent> &event) {

      auto inputhits = event->Get<edm4hep::SimTrackerHit>(m_readout);
      auto outputTracks = m_reco_algo.produce(inputhits);
      Set(outputTracks);

    }

}
