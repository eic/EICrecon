// Created by Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

// Event Model related classes
#include <edm4eic/TrackParametersCollection.h>
#include <algorithms/fardetectors/FarDetectorLinearTracking.h>

#include <extensions/jana/JChainMultifactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>

namespace eicrecon {

    class FarDetectorLinearTracking_factoryT :
    public JChainMultifactoryT<FarDetectorLinearTrackingConfig>,
    public SpdlogMixin {

    public:

      explicit FarDetectorLinearTracking_factoryT(
	  std::string tag,
          const std::vector<std::string>& input_tags,
          const std::vector<std::string>& output_tags,
	  FarDetectorLinearTrackingConfig cfg):
	  JChainMultifactoryT(std::move(tag), input_tags, output_tags, cfg) {

	  DeclarePodioOutput<edm4eic::TrackParameters>(GetOutputTags()[0]);
      }

        /** One time initialization **/
        void Init() override {
	  auto app = GetApplication();

	  // SpdlogMixin logger initialization, sets m_log
	  InitLogger(app, GetPrefix(), "info");

	  auto cfg = GetDefaultConfig();

	  m_geoSvc = app->GetService<JDD4hep_service>();

	  if (cfg.detconf.readout.empty()) {
	    throw JException("Readout is empty");
	  }

	  try {
	    id_dec = m_geoSvc->detector()->readout(cfg.detconf.readout).idSpec().decoder();
	    if (!cfg.detconf.moduleField.empty()) {
	      cfg.detconf.module_idx = id_dec->index(cfg.detconf.moduleField);
	      logger()->debug("Find module field {}, index = {}", cfg.detconf.moduleField, cfg.detconf.module_idx);
	    }
	    if (!cfg.detconf.layerField.empty()) {
	      cfg.detconf.layer_idx = id_dec->index(cfg.detconf.layerField);
	      logger()->debug("Find layer field {}, index = {}", cfg.detconf.layerField, cfg.detconf.layer_idx);
	    }
	    if (!cfg.detconf.xField.empty()) {
	      cfg.detconf.x_idx = id_dec->index(cfg.detconf.xField);
	      logger()->debug("Find layer field {}, index = {}",  cfg.detconf.xField, cfg.detconf.x_idx);
	    }
	    if (!cfg.detconf.yField.empty()) {
	      cfg.detconf.y_idx = id_dec->index(cfg.detconf.yField);
	      logger()->debug("Find layer field {}, index = {}", cfg.detconf.yField, cfg.detconf.y_idx);
	    }
	  } catch (...) {
	    logger()->error("Failed to load ID decoder for {}", cfg.detconf.readout);
	    throw JException("Failed to load ID decoder");
	  }

	  m_reco_algo.applyConfig(cfg);
	  m_reco_algo.init(id_dec,logger());

	}

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override {

	  auto inputhits = static_cast<const edm4hep::TrackerHitCollection*>(event->GetCollectionBase(GetInputTags()[0]));

	  try {
	    auto outputTracks = m_reco_algo.produce(*inputhits);
	    SetCollection<edm4eic::TrackParameters>(GetOutputTags()[0],std::move(outputTracks));
	  }
	  catch(std::exception &e) {
	    throw JException(e.what());
	  }

	}

    private:

	eicrecon::FarDetectorLinearTracking m_reco_algo;        // Actual digitisation algorithm

	dd4hep::BitFieldCoder *id_dec{nullptr};
	std::shared_ptr<JDD4hep_service> m_geoSvc;

    };

} // eicrecon
