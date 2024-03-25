// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#pragma once

#include <spdlog/spdlog.h>
#include <DD4hep/Detector.h>
#include <algorithms/algorithm.h>
#include <string>
#include <string_view>

#include "services/log/Log_service.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

   template<class T, class TA>
     using FilterByAssociationsAlgorithm =  algorithms::Algorithm<
       typename algorithms::Input<
        typename T::collection_type,
        typename TA::collection_type
        >,
       typename algorithms::Output<
        typename TA::collection_type,
        typename TA::collection_type
        >
     >;


  template<typename T,typename TA, auto MemberFunctionPtr>
  class FilterByAssociations : public FilterByAssociationsAlgorithm<T,TA> {

    public:
    FilterByAssociations(std::string_view name)
      : FilterByAssociationsAlgorithm<T,TA>{name,
                        {"inputCollection","associatedCollection"},
                        {"isAssociatedCollection","isNotAssociatedCollection"},
                        "Filter associated collection"
                      } {
        };

        void init() final { };

        void process(const typename FilterByAssociationsAlgorithm<T,TA>::Input& input, const typename FilterByAssociationsAlgorithm<T,TA>::Output& output) const final{

          const auto [entries,associatedEntries] = input;
          auto [is_associated,is_not_associated] = output;

          is_associated->setSubsetCollection();
          is_not_associated->setSubsetCollection();

          for (const auto& entry : *entries) {

            for (const auto& associatedEntries : *associatedEntries){
              if(entry == (associatedEntries.*MemberFunctionPtr)()){
                is_associated->push_back(associatedEntries);
              }
              else{
                is_not_associated->push_back(associatedEntries);
              }
            }

          }

        };

  };

} // eicrecon
