// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#pragma once

#include <spdlog/spdlog.h>
#include <DD4hep/Detector.h>
#include <algorithms/algorithm.h>
#include <string>
#include <string_view>

#include "services/log/Log_service.h"
#include "algorithms/meta/SubDivideCollectionConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

template <class T>
using SubDivideCollectionAlgorithm =
    algorithms::Algorithm<typename algorithms::Input<const typename T::collection_type>,
                          typename algorithms::Output<std::vector<typename T::collection_type>>>;

template <typename T>
class SubDivideCollection : public SubDivideCollectionAlgorithm<T>,
                            public WithPodConfig<SubDivideCollectionConfig<T>> {

public:
  SubDivideCollection(std::string_view name)
      : SubDivideCollectionAlgorithm<T>{name,
                                        {"inputCollection"},
                                        {"outputCollection"},
                                        "Sub-Divide collection"}
      , WithPodConfig<SubDivideCollectionConfig<T>>() {};

  void init() final {};

  void process(const typename SubDivideCollectionAlgorithm<T>::Input& input,
               const typename SubDivideCollectionAlgorithm<T>::Output& output) const final {

    const auto [entries]      = input;
    auto [subdivided_entries] = output;

    for (auto out : subdivided_entries) {
      out->setSubsetCollection();
    }

    for (const auto& entry : *entries) {

      auto div_indices = this->m_cfg.function(entry);

      for (auto index : div_indices) {
        subdivided_entries[index]->push_back(entry);
      }
    }

    //Log how the hits were divided between the collection names
    this->debug("Divided {} hits between {} collections", entries->size(),
                subdivided_entries.size());
    //How many hits in each output collection
    for (std::size_t i = 0; i < subdivided_entries.size(); i++) {
      this->debug("Collection {} takes {} hits", i, subdivided_entries[i]->size());
    }
  };
};

} // namespace eicrecon
