// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#include "SplitGeometry.h"

#include "algorithms/digi/SplitGeometryConfig.h"

#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include <gsl/pointers>

namespace eicrecon {

template<class T>
void SplitGeometry<T>::init(const dd4hep::Detector* detector, std::shared_ptr<spdlog::logger>& logger) {
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

template<class T>
void SplitGeometry<T>::process(const typename SplitGeometryAlgorithm<T>::Input& input,
                               const typename SplitGeometryAlgorithm<T>::Output& output) const {

  const auto [hits]      = input;
  auto [subdivided_hits] = output;

  for(int i=0; i<m_cfg.divisions.size(); i++){
    subdivided_hits.push_back(std::make_unique<SplitGeometry::Input>());
    //subdivided_hits.push_back(std::make_unique<SplitGeometry::Input>());
    subdivided_hits.back()->setSubsetCollection();
  }

  for (auto hit : hits) {
    auto cellID  = hit->getCellID();
    int division = m_id_dec->get( cellID, m_division_idx );

    auto div_index = std::find(m_cfg.divisions.begin(),m_cfg.divisions.end(),division); 

    if(div_index != m_cfg.divisions.end()){
      int index = div_index-m_cfg.divisions.begin();
      subdivided_hits[index]->push_back(hit);
    } else {
      m_log->debug("Hit division not requested as output = {}", division);      
    }

  }
  
}

} // namespace eicrecon
