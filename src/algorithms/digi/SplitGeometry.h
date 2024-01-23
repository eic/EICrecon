// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#pragma once

#include <edm4eic/RawTrackerHitCollection.h>
#include <spdlog/spdlog.h>
#include <DD4hep/Detector.h>
#include <algorithms/algorithm.h>
#include <string>
#include <string_view>

#include "services/log/Log_service.h"
#include "SplitGeometryConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

//    using SplitGeometryAlgorithm =  algorithms::Algorithm<
//    algorithms::Input<const edm4eic::RawTrackerHitCollection>,
//    algorithms::Output<std::vector<edm4eic::RawTrackerHitCollection>>
//    >;

   template<class T> 
     using SplitGeometryAlgorithm =  algorithms::Algorithm< 
     typename algorithms::Input<const T>, 
     typename algorithms::Output<std::vector<T>> 
     >; 


  template<class T>
  class SplitGeometry : public SplitGeometryAlgorithm<T>, public WithPodConfig<SplitGeometryConfig>  {

    public:
    SplitGeometry(std::string_view name) 
      : SplitGeometryAlgorithm<T>{name,
		      {"inputHitCollection"},
			{"outputCollection"},
			  "Divide hit collection by dd4hep field id"
		      }{}

        void init(const dd4hep::Detector* detector,std::shared_ptr<spdlog::logger>& logger){ // set logger
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

        };
  
	      void process(const typename SplitGeometry::Input& input, const typename SplitGeometryAlgorithm<T>::Output& output) const final{

          const auto [hits]      = input;
          auto [subdivided_hits] = output;

          for( auto out : subdivided_hits){
            out->setSubsetCollection();
          }

          for (const auto& hit : *hits) {
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
          
        };

    private:
        const dd4hep::Detector*         m_detector{nullptr};
        const dd4hep::BitFieldCoder*    m_id_dec{nullptr};
        std::shared_ptr<spdlog::logger> m_log;

	      int m_division_idx;

  };

} // eicrecon
