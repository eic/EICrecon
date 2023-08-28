// Created by Alex Jentsch to do Roman Pots reconstruction
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include "MatrixTransferStatic_factory.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"

namespace eicrecon {

    void MatrixTransferStatic_factory::Init() {

	auto app = GetApplication();

	m_log = app->GetService<Log_service>()->logger(GetTag());

	auto cfg = GetDefaultConfig();

	m_geoSvc = app->GetService<JDD4hep_service>();

	m_readout = GetDefaultInputTags()[0];

	if (m_readout.empty()) {
	  throw JException("Readout is empty");
	}


	m_geoSvc = app->GetService<JDD4hep_service>();

    }


    void MatrixTransferStatic_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    	// Nothing to do here
    }

    void MatrixTransferStatic_factory::Process(const std::shared_ptr<const JEvent> &event) {

      auto inputhits = static_cast<const edm4hep::SimTrackerHitCollection*>(event->GetCollectionBase(GetDefaultInputTags()[0]));

      try {
	auto outputTracks = m_reco_algo.produce(*inputhits);
	SetCollection(std::move(outputTracks));
      }
      catch(std::exception &e) {
	throw JException(e.what());
      }

    }
}
