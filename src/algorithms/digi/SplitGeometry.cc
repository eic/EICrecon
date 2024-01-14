// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#include "SplitGeometry.h"

#include "algorithms/digi/SplitGeometryConfig.h"

#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"

namespace eicrecon {

void SplitGeometry::init(const dd4hep::Detector* detector, std::shared_ptr<spdlog::logger>& logger) {
    // set logger
    m_log      = logger;
    m_detector = detector;

    if (m_cfg.readout.empty()) {
      throw JException("Readout is empty");
    }

    if (m_cfg.division.empty()) {
      throw JException("Readout devision is undefined");
    }

    try {
      m_id_dec = m_detector->readout(m_cfg.readout).idSpec().decoder();
      m_division_idx = m_id_dec->index(m_cfg.division);
      m_log->debug("Find division field {}, index = {}", m_cfg.division, m_division_idx);    
    } catch (...) {
      m_log->error("Failed to load ID decoder for {}", m_cfg.readout);
      throw JException("Failed to load ID decoder");
    }

}

std::vector<std::unique_ptr<edm4eic::RawTrackerHitCollection>>
SplitGeometry::process(const edm4eic::RawTrackerHitCollection& inputhits) {

  std::vector<std::unique_ptr<edm4eic::RawTrackerHitCollection>> subdivided_hits(m_cfg.divisions.size());

  //, std::make_unique<edm4eic::RawTrackerHitCollection>()

  for(auto& collection : subdivided_hits){
    collection = std::make_unique<edm4eic::RawTrackerHitCollection>();
    collection->setSubsetCollection();
  }

  for (auto hit : inputhits) {
    auto cellID  = hit.getCellID();
    int division = m_id_dec->get( cellID, m_division_idx );

    auto div_index = std::find(m_cfg.divisions.begin(),m_cfg.divisions.end(),division); 

    if(div_index != m_cfg.divisions.end()){
      int index = div_index-m_cfg.divisions.begin();
      subdivided_hits[index]->push_back(hit);
    } else {
      m_log->debug("Hit division not requested as output = {}", division);      
    }

  }
  return std::move(subdivided_hits);

}

} // namespace eicrecon
