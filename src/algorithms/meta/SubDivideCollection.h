// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#pragma once

#include <spdlog/spdlog.h>
#include <DD4hep/Detector.h>
#include <algorithms/algorithm.h>
#include <string>
#include <string_view>

#include "services/log/Log_service.h"
#include "SubDivideCollectionConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

   template<class T>
     using SubDivideCollectionAlgorithm =  algorithms::Algorithm<
     typename algorithms::Input<const typename T::collection_type>,
     typename algorithms::Output<std::vector<typename T::collection_type>>
     >;


  template<typename T>
  class SubDivideCollection : public SubDivideCollectionAlgorithm<T>, public WithPodConfig<SubDivideCollectionConfig<T>>  {

    public:
    SubDivideCollection(std::string_view name)
      : SubDivideCollectionAlgorithm<T>{name,
                      {"inputCollection"},
                        {"outputCollection"},
                          "Sub-Divide collection"
                      },
        WithPodConfig<SubDivideCollectionConfig<T>>() {
        };

        void init(std::shared_ptr<spdlog::logger>& logger){ // set logger
          m_log      = logger;
        };

        void process(const typename SubDivideCollectionAlgorithm<T>::Input& input, const typename SubDivideCollectionAlgorithm<T>::Output& output) const final{

          const auto [entries]      = input;
          auto [subdivided_entries] = output;

          for( auto out : subdivided_entries){
            out->setSubsetCollection();
          }

          for (const auto& entry : *entries) {

            auto div_indecies = this->m_cfg.function(entry);

            for (auto index : div_indecies){
              subdivided_entries[index]->push_back(entry);
            }

          }

        };

    private:
      std::shared_ptr<spdlog::logger> m_log;

  };

} // eicrecon
