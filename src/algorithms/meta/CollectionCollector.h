// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024-2025 Simon Gardner

#include <spdlog/spdlog.h>
#include <algorithms/algorithm.h>
#include <string>
#include <string_view>

#include "services/log/Log_service.h"
#include "algorithms/meta/CollectionCollectorConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

template <class T>
using CollectionCollectorAlgorithm =
    algorithms::Algorithm<typename algorithms::Input<std::vector<const T>>,
                          typename algorithms::Output<T>>;

template <class T>
class CollectionCollector : public CollectionCollectorAlgorithm<T>,
                            public WithPodConfig<CollectionCollectorConfig> {

public:
  CollectionCollector(std::string_view name)
      : CollectionCollectorAlgorithm<T>{name,
                                        {"inputCollections"},
                                        {"outputCollection"},
                                        "Merge content of collections into one subset collection"}
      , WithPodConfig<CollectionCollectorConfig>(){};

  void init() final{};

  void process(const typename CollectionCollector::Input& input,
               const typename CollectionCollector::Output& output) const final {

    const auto [in_collections] = input;
    auto [out_collection]       = output;

    if (!this->m_cfg.output_copies) {
      out_collection->setSubsetCollection();
    }

    for (const auto& collection : in_collections) {
      for (const auto& entry : *collection) {
        if (this->m_cfg.output_copies) {
          // Create a copy of the entry and add it to the output collection
          out_collection->push_back(entry.clone());
        } else {
          // Add the entry to the subcollection
          out_collection->push_back(entry);
        }
      }
    }
    //Log how many entries were collected from N input collections
    this->debug("Collected {} entries from {} input collections", out_collection->size(),
                in_collections.size());
  }
};
} // namespace eicrecon
