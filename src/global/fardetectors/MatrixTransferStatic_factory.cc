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

	auto app = GetApplication();

	m_log = app->GetService<Log_service>()->logger(GetTag());

	auto cfg = GetDefaultConfig();

	m_geoSvc = app->GetService<JDD4hep_service>();

	if (cfg.detconf.readout.empty()) {
	  throw JException("Readout is empty");
	}

	m_readout = GetDefaultInputTags()[0];

	m_geoSvc = app->GetService<JDD4hep_service>();

    }


    void FarDetectorReconstruction_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    	// Nothing to do here
    }

    void FarDetectorReconstruction_factory::Process(const std::shared_ptr<const JEvent> &event) {

      auto inputhits = static_cast<const edm4hep::SimTrackerHitCollection*>(event->GetCollectionBase(GetDefaultInputTags()[0]));

      try {
	auto outputTracks = m_reco_algo.produce(*inputhits);
	SetCollection(std::move(outputTracks));
      }
      catch(std::exception &e) {
	throw JException(e.what());
      }

}
