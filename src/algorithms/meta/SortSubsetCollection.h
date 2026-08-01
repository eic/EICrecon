// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, ePIC Collaboration

#pragma once

#include <algorithms/algorithm.h>
#include <algorithm>
#include <functional>
#include <string_view>
#include <utility>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"
#include "services/log/Log_service.h"

namespace eicrecon {

template <class T>
using SortSubsetCollectionAlgorithm =
    algorithms::Algorithm<typename algorithms::Input<const typename T::collection_type>,
                          typename algorithms::Output<typename T::collection_type>>;

template <class T, class AccessorT>
class SortSubsetCollection : public SortSubsetCollectionAlgorithm<T>,
                             public WithPodConfig<NoConfig> {

public:
  SortSubsetCollection(std::string_view name, AccessorT accessor)
      : SortSubsetCollectionAlgorithm<T>{name,
                                         {"inputCollection"},
                                         {"outputCollection"},
                                         "Sort collection into a subset output collection"}
      , m_accessor(std::move(accessor)) {}

  void init() final {};

  void process(const typename SortSubsetCollectionAlgorithm<T>::Input& input,
               const typename SortSubsetCollectionAlgorithm<T>::Output& output) const final {
    const auto [input_collection] = input;
    auto [output_collection]      = output;

    output_collection->setSubsetCollection();

    std::vector<T> sorted_entries;
    sorted_entries.reserve(input_collection->size());
    for (const auto& entry : *input_collection) {
      sorted_entries.push_back(entry);
    }

    std::sort(sorted_entries.begin(), sorted_entries.end(),
              [this](const auto& lhs, const auto& rhs) {
                const auto lhs_key = std::invoke(m_accessor, lhs);
                const auto rhs_key = std::invoke(m_accessor, rhs);
                if (lhs_key < rhs_key) {
                  return true;
                }
                if (rhs_key < lhs_key) {
                  return false;
                }

                const auto lhs_id = lhs.getObjectID();
                const auto rhs_id = rhs.getObjectID();
                if (lhs_id.collectionID < rhs_id.collectionID) {
                  return true;
                }
                if (rhs_id.collectionID < lhs_id.collectionID) {
                  return false;
                }
                return lhs_id.index < rhs_id.index;
              });

    for (const auto& entry : sorted_entries) {
      output_collection->push_back(entry);
    }
  };

private:
  AccessorT m_accessor;
};

} // namespace eicrecon
