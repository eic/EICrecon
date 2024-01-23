// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#pragma once

#include <edm4eic/RawTrackerHitCollection.h>
#include <spdlog/spdlog.h>
#include <DD4hep/Detector.h>
#include <algorithms/algorithm.h>
#include <string>
#include <string_view>

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

        void init(const dd4hep::Detector* detector,std::shared_ptr<spdlog::logger>& logger);

	void process(const typename SplitGeometryAlgorithm<T>::Input&, const typename SplitGeometryAlgorithm<T>::Output&) const final;

    private:
        const dd4hep::Detector*         m_detector{nullptr};
        const dd4hep::BitFieldCoder*    m_id_dec{nullptr};
        std::shared_ptr<spdlog::logger> m_log;

	int m_division_idx;

  };

} // eicrecon
