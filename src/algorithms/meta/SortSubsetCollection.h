// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, ePIC Collaboration

#pragma once

#include <algorithms/algorithm.h>
#include <algorithm>
#include <functional>
#include <numeric>
#include <podio/ObjectID.h>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"
#include "services/log/Log_service.h"

namespace eicrecon {

template <class T>
using SortSubsetCollectionAlgorithm =
    algorithms::Algorithm<typename algorithms::Input<const typename T::collection_type>,
                          typename algorithms::Output<typename T::collection_type>>;

template <class T, class AccessorFunctionT>
class SortSubsetCollection : public SortSubsetCollectionAlgorithm<T>,
                             public WithPodConfig<NoConfig> {

public:
  SortSubsetCollection(std::string_view name, AccessorFunctionT accessor)
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

    const std::size_t n = input_collection->size();

    // Pre-calculate sort keys once per entry to avoid O(N log N) accessor invocations
    using KeyT = std::decay_t<std::invoke_result_t<AccessorFunctionT, T>>;
    std::vector<KeyT> keys;
    keys.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
      keys.push_back(std::invoke(m_accessor, (*input_collection)[i]));
    }

    // Obtain sorted permutation of indices using pre-calculated keys
    std::vector<std::size_t> indices(n);
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&](std::size_t i, std::size_t j) {
      if (keys[i] < keys[j]) {
        return true;
      }
      if (keys[j] < keys[i]) {
        return false;
      }
      const auto id_i = (*input_collection)[i].getObjectID();
      const auto id_j = (*input_collection)[j].getObjectID();
      if (id_i.collectionID < id_j.collectionID) {
        return true;
      }
      if (id_j.collectionID < id_i.collectionID) {
        return false;
      }
      return id_i.index < id_j.index;
    });

    for (std::size_t i : indices) {
      output_collection->push_back((*input_collection)[i]);
    }
  };

private:
  AccessorFunctionT m_accessor;
};

} // namespace eicrecon
