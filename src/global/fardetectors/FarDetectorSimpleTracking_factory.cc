// Created by Simon Gardner to do LowQ2 tracking
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include <edm4hep/Vector2f.h>
#include <edm4eic/Cov2f.h>
#include <edm4eic/Cov3f.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4hep/TrackerHitCollection.h>

#include "FarDetectorSimpleTracking_factory.h"
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>
#include <ROOT/RVec.hxx>
#include <TDecompSVD.h>
#include <TMatrixD.h>


namespace eicrecon {


    void FarDetectorSimpleTracking_factory::Init() {

	auto app = GetApplication();

	m_log = app->GetService<Log_service>()->logger(GetTag());

	auto cfg = GetDefaultConfig();

	m_geoSvc = app->GetService<JDD4hep_service>();

	if (cfg.detconf.readout.empty()) {
	  throw JException("Readout is empty");
	}
	
	try {
	  id_dec = m_geoSvc->detector()->readout(cfg.detconf.readout).idSpec().decoder();
	  if (!cfg.detconf.moduleField.empty()) {
	    cfg.detconf.module_idx = id_dec->index(cfg.detconf.moduleField);
	    m_log->debug("Find module field {}, index = {}", cfg.detconf.moduleField, cfg.detconf.module_idx);
	  }
	  if (!cfg.detconf.layerField.empty()) {
	    cfg.detconf.layer_idx = id_dec->index(cfg.detconf.layerField);
	    m_log->debug("Find layer field {}, index = {}", cfg.detconf.layerField, cfg.detconf.layer_idx);
	  }
	  if (!cfg.detconf.xField.empty()) {
	    cfg.detconf.x_idx = id_dec->index(cfg.detconf.xField);
	    m_log->debug("Find layer field {}, index = {}",  cfg.detconf.xField, cfg.detconf.x_idx);
	  }
	  if (!cfg.detconf.yField.empty()) {
	    cfg.detconf.y_idx = id_dec->index(cfg.detconf.yField);
	    m_log->debug("Find layer field {}, index = {}", cfg.detconf.yField, cfg.detconf.y_idx);
	  }
	} catch (...) {
	  m_log->error("Failed to load ID decoder for {}", cfg.detconf.readout);
	  throw JException("Failed to load ID decoder");
	}
	m_reco_algo.setEncoder(id_dec);
	m_reco_algo.applyConfig(cfg);
	
    }


    void FarDetectorSimpleTracking_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    	// Nothing to do here
    }

    void FarDetectorSimpleTracking_factory::Process(const std::shared_ptr<const JEvent> &event) {
    

        auto inputhits = static_cast<const edm4hep::TrackerHitCollection*>(event->GetCollectionBase(GetDefaultInputTags()[0]));


        try {
	  auto outputTracks = m_reco_algo.produce(*inputhits);
	  SetCollection(std::move(outputTracks));
	}
	catch(std::exception &e) {
	  throw JException(e.what());
	}

    }

}
