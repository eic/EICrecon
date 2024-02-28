// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#include <spdlog/spdlog.h>
#include <algorithms/algorithm.h>
#include <string>
#include <string_view>

#include "services/log/Log_service.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

  template<class T>
    using CollectionCollectorAlgorithm =  algorithms::Algorithm<
      typename algorithms::Input<std::vector<const T>>,
      typename algorithms::Output<T>
    >;

  template<class T>
  class CollectionCollector : public CollectionCollectorAlgorithm<T>  {

    public:
    CollectionCollector(std::string_view name)
      : CollectionCollectorAlgorithm<T>{name,
                      {"inputCollections"},
                        {"outputCollection"},
                          "Merge content of collections into one subset collection"
                      }{}

    void init(std::shared_ptr<spdlog::logger>& logger){ // set logger
      m_log      = logger;
    };

    void process(const typename CollectionCollector::Input& input, const typename CollectionCollector::Output& output) const final{

      const auto [in_collections] = input;
      auto [out_collection]       = output;

      out_collection->setSubsetCollection();

      for (const auto& collection : in_collections) {
        for (const auto& hit : *collection) {
          out_collection->push_back(hit);
        }
      }
    }

    private:
      std::shared_ptr<spdlog::logger> m_log; // logger

  };
} // eicrecon
