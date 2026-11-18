// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#pragma once

#include <gsl/pointers>
#include <memory>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/meta/CollectionCollector.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

/**
 * Factory that selects the first available collection from a list of optional inputs,
 * and outputs it as a subset collection.
 *
 * This allows falling back to alternative sources for the same information, e.g. to use
 * the truth-matched scattered electron when MC truth is available, and a reconstructed
 * candidate otherwise. Note that an available but empty collection is selected over later
 * inputs (unlike CollectionCollector_factory, which concatenates all available inputs).
 *
 * Template parameter T is the PODIO object type (e.g., edm4eic::ReconstructedParticle)
 *
 * Example usage in plugin:
 *   app->Add(new JOmniFactoryGeneratorT<
 *            FirstAvailableCollection_factory<edm4eic::ReconstructedParticle>>(
 *       "ScatteredElectronsForKinematics",
 *       {"ScatteredElectronsTruth", "ScatteredElectronsEMinusPz"},
 *       {"ScatteredElectronsForKinematics"},
 *       app
 *   ));
 */
template <class T>
class FirstAvailableCollection_factory
    : public JOmniFactory<FirstAvailableCollection_factory<T>, NoConfig> {
public:
  using FactoryT = JOmniFactory<FirstAvailableCollection_factory<T>, NoConfig>;
  using AlgoT    = eicrecon::CollectionCollector<typename T::collection_type>;

private:
  std::unique_ptr<AlgoT> m_algo;

  typename FactoryT::template VariadicPodioInput<T, true> m_inputs{this};
  typename FactoryT::template PodioOutput<T> m_output{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(this->logger()->level()));
    m_algo->applyConfig(this->config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    // Missing optional inputs are skipped or returned as nullptr; select the first available
    std::vector<gsl::not_null<const typename T::collection_type*>> in_collections;
    for (const auto& in_collection : m_inputs()) {
      if (in_collection != nullptr) {
        in_collections.push_back(gsl::not_null<const typename T::collection_type*>{in_collection});
        break;
      }
    }
    m_algo->process(in_collections, m_output().get());
  };
};

} // namespace eicrecon
