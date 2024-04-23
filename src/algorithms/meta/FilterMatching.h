// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#pragma once

#include <spdlog/spdlog.h>
#include <algorithms/algorithm.h>
#include <string>
#include <string_view>

#include "services/log/Log_service.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

   template<class ToFilterObjectT, class FilterByObjectT>
     using FilterMatchingAlgorithm =  algorithms::Algorithm<
       typename algorithms::Input<
        typename ToFilterObjectT::collection_type,
        typename FilterByObjectT::collection_type
        >,
       typename algorithms::Output<
        typename ToFilterObjectT::collection_type,
        typename ToFilterObjectT::collection_type
        >
     >;

  /// Filters a collection by the members of another collection
  /// The first collection is divided up into two collections where its elements either passed the filter or not
  /// The second collection provides a filter
  /// Functions need to be provided along with the collections to form the link between the data types
  /// These functions are envisioned to link the objectIDs of the collection/associations but could be anything
  template<typename ToFilterObjectT,auto ToFilterFunction, typename FilterByObjectT,auto FilterByFunction>
  class FilterMatching : public FilterMatchingAlgorithm<ToFilterObjectT,FilterByObjectT> {

    public:
    FilterMatching(std::string_view name)
      : FilterMatchingAlgorithm<ToFilterObjectT,FilterByObjectT>{name,
                        {"inputCollection","inputAssociatedCollection"},
                        {"outputMatchedAssociations","outputUnmatchedAssociations"},
                        "Filter associated collection"
                      } {
        };

        void init() final { };

        void process(const typename FilterMatchingAlgorithm<ToFilterObjectT,FilterByObjectT>::Input& input,
                    const typename FilterMatchingAlgorithm<ToFilterObjectT,ToFilterObjectT>::Output& output) const final{

          const auto [toFilterEntries,filterByEntries] = input;
          auto [is_associated,is_not_associated] = output;

          is_associated->setSubsetCollection();
          is_not_associated->setSubsetCollection();

          for (const auto& associatedEntry : *toFilterEntries){

            auto associationID = ToFilterFunction(&associatedEntry);

            bool foundAssociation = false;

            // Tries to find the association in the entries
            for(const auto& entry : *filterByEntries){
              auto objectID = FilterByFunction(&entry);
              if(objectID == associationID){
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
