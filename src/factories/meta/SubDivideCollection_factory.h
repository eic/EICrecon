// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#pragma once

#include "algorithms/meta/SubDivideCollection.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"


namespace eicrecon {

template <class T>
class SubDivideCollection_factory : public JOmniFactory<SubDivideCollection_factory<T>, SubDivideCollectionConfig<T>> {

  public:
    using AlgoT = eicrecon::SubDivideCollection<T>;

  private:

    std::unique_ptr<AlgoT> m_algo;

    typename JOmniFactory<SubDivideCollection_factory<T>, SubDivideCollectionConfig<T>>::template PodioInput<T> m_raw_hits_input {this};
    typename JOmniFactory<SubDivideCollection_factory<T>, SubDivideCollectionConfig<T>>::template VariadicPodioOutput<T> m_split_hits_output {this};

    typename JOmniFactory<SubDivideCollection_factory<T>, SubDivideCollectionConfig<T>>::template Service<DD4hep_service> m_geoSvc {this};

public:
    void Configure() {
      m_algo = std::make_unique<AlgoT>(this->GetPrefix());
      m_algo->applyConfig(this->config());
      m_algo->init(this->logger());
    }

    void setFunctions(std::function<std::vector<int>(const T&)> f) {
      m_algo->setFunction(f);
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {

    try {
      const typename T::collection_type* raw_hits = m_raw_hits_input();
      std::vector<gsl::not_null<typename T::collection_type*>> split_hits;
      for (const auto& split_hit : m_split_hits_output()) {
        split_hits.push_back(gsl::not_null<typename T::collection_type*>(split_hit.get()));
      }
      m_algo->process(raw_hits,split_hits);
    }
    catch(std::exception &e) {
      logger()->warn(e.what());
      throw JException(e.what());
    }
};
}; // SplitGeometry_factory
} // eicrecon
