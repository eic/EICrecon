// Created by Simon Gardner to do LowQ2 pixel clustering
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include <edm4eic/RawTrackerHitCollection.h>

#include "FarDetectorCluster_factory.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"

namespace eicrecon {

  void FarDetectorCluster_factory::Init() {

    auto app = GetApplication();

    m_log = app->GetService<Log_service>()->logger(GetTag());

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
    m_reco_algo.setGeoConverter(m_geoSvc->cellIDPositionConverter());
    m_reco_algo.setEncoder(id_dec);
    m_reco_algo.applyConfig(cfg);

  }


  void FarDetectorCluster_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    // Nothing to do here
  }


  void FarDetectorCluster_factory::Process(const std::shared_ptr<const JEvent> &event) {

    auto inputhits  = static_cast<const edm4eic::RawTrackerHitCollection*>(event->GetCollectionBase(GetDefaultInputTags()[0]));

  try {
    auto outputclusters = m_reco_algo.produce(*inputhits);
    SetCollection(std::move(outputclusters));
  }
  catch(std::exception &e) {
    throw JException(e.what());
  }

  }

}


