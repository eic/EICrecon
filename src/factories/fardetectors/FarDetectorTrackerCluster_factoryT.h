// Created by Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once
#include <services/geometry/dd4hep/JDD4hep_service.h>

// Event Model related classes
#include <edm4hep/TrackerHitCollection.h>
#include <edm4eic/RawTrackerHit.h>
#include <algorithms/fardetectors/FarDetectorTrackerCluster.h>
#include <algorithms/fardetectors/FarDetectorTrackerClusterConfig.h>

#include <extensions/jana/JChainMultifactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>

namespace eicrecon {

  class FarDetectorTrackerCluster_factoryT :
  public JChainMultifactoryT<FarDetectorTrackerClusterConfig>,
  public SpdlogMixin {

  public:

    explicit FarDetectorTrackerCluster_factoryT(
	  std::string tag,
          const std::vector<std::string>& input_tags,
          const std::vector<std::string>& output_tags,
	  FarDetectorTrackerClusterConfig cfg ):
          JChainMultifactoryT<FarDetectorTrackerClusterConfig>(std::move(tag),input_tags,output_tags,cfg ) {

      DeclarePodioOutput<edm4hep::TrackerHit>(GetOutputTags()[0]);

    }


      /** One time initialization **/
      void Init() override {

	auto app = GetApplication();

	// SpdlogMixin logger initialization, sets m_log
	InitLogger(app, GetPrefix(), "info");

	auto cfg = GetDefaultConfig();

	m_geoSvc = app->GetService<JDD4hep_service>();

	if (cfg.readout.empty()) {
	  throw JException("Readout is empty");
	}

	try {
	  id_dec = m_geoSvc->detector()->readout(cfg.readout).idSpec().decoder();
	  if (!cfg.moduleField.empty()) {
	    cfg.module_idx = id_dec->index(cfg.moduleField);
	    m_log->debug("Find module field {}, index = {}", cfg.moduleField, cfg.module_idx);
	  }
	  if (!cfg.layerField.empty()) {
	    cfg.layer_idx = id_dec->index(cfg.layerField);
	    m_log->debug("Find layer field {}, index = {}", cfg.layerField, cfg.layer_idx);
	  }
	  if (!cfg.xField.empty()) {
	    cfg.x_idx = id_dec->index(cfg.xField);
	    m_log->debug("Find layer field {}, index = {}",  cfg.xField, cfg.x_idx);
	  }
	  if (!cfg.yField.empty()) {
	    cfg.y_idx = id_dec->index(cfg.yField);
	    m_log->debug("Find layer field {}, index = {}", cfg.yField, cfg.y_idx);
	  }
	} catch (...) {
	  m_log->error("Failed to load ID decoder for {}", cfg.readout);
	  throw JException("Failed to load ID decoder");
	}

	// Setup algorithm
	m_reco_algo.applyConfig(cfg);
	m_reco_algo.init(m_geoSvc->cellIDPositionConverter(),id_dec,logger());

      }



      /** Event by event processing **/
      void Process(const std::shared_ptr<const JEvent> &event) override {
	auto inputhits  = static_cast<const edm4eic::RawTrackerHitCollection*>(event->GetCollectionBase(GetInputTags()[0]));

	try {
	  auto outputclusters = m_reco_algo.produce(*inputhits);
	  SetCollection<edm4hep::TrackerHit>(GetOutputTags()[0],std::move(outputclusters));
	}
	catch(std::exception &e) {
	  throw JException(e.what());
	}

      }


  private:
      eicrecon::FarDetectorTrackerCluster m_reco_algo;        // Actual digitisation algorithm

      dd4hep::BitFieldCoder *id_dec{nullptr};
      std::shared_ptr<JDD4hep_service> m_geoSvc;

  };

} // eicrecon
