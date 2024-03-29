// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

// Filters a collection by the members of another collection
// The first collection is the collection which provides a filter
// The second is the one which is divided up into two collections where its elements either passed the filter or not
// Functions need to be provided along with the collections to form the link between the data types
// These functions are envisioned to link the objectIDs of the collection/associations but could be anything

#pragma once

#include <spdlog/spdlog.h>
#include <algorithms/algorithm.h>
#include <string>
#include <string_view>

#include "services/log/Log_service.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

   template<class FilterByObjectT, class ToFilterObjectT>
     using FilterByAssociationsAlgorithm =  algorithms::Algorithm<
       typename algorithms::Input<
        typename FilterByObjectT::collection_type,
        typename ToFilterObjectT::collection_type
        >,
       typename algorithms::Output<
        typename ToFilterObjectT::collection_type,
        typename ToFilterObjectT::collection_type
        >
     >;


  template<typename FilterByObjectT,auto FilterByFunction,typename ToFilterObjectT,auto ToFilterFunction>
  class FilterByAssociations : public FilterByAssociationsAlgorithm<FilterByObjectT,ToFilterObjectT> {

    public:
    FilterByAssociations(std::string_view name)
      : FilterByAssociationsAlgorithm<FilterByObjectT,ToFilterObjectT>{name,
                        {"inputCollection","associatedCollection"},
                        {"isAssociatedCollection","isNotAssociatedCollection"},
                        "Filter associated collection"
                      } {
        };

        void init() final { };

        void process(const typename FilterByAssociationsAlgorithm<FilterByObjectT,ToFilterObjectT>::Input& input,
                    const typename FilterByAssociationsAlgorithm<FilterByObjectT,ToFilterObjectT>::Output& output) const final{

          const auto [entries,associatedEntries] = input;
          auto [is_associated,is_not_associated] = output;

          is_associated->setSubsetCollection();
          is_not_associated->setSubsetCollection();

          for (const auto& associatedEntry : *associatedEntries){

            auto associationID = ToFilterFunction(&associatedEntry);

            bool foundAssociation = false;

            // Tries to find the association in the entries
            for(const auto& entry : *entries){
              auto objectID = FilterByFunction(&entry);
              if(entry.getObjectID() == associationID){
                is_associated->push_back(associatedEntry);
                foundAssociation = true;
                break;
              }
            }

            if(!foundAssociation){
              is_not_associated->push_back(associatedEntry);
            }

          }

        };

  };

} // eicrecon
