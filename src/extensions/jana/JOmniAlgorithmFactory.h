// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#pragma once

/**
 * JOmniAlgorithmFactory: a JOmniFactory whose input and output collection handles,
 * Configure() and Process() are derived from the algorithms::Algorithm<Input<...>, Output<...>>
 * signature of the wrapped algorithm. This is the JANA analog of Jug::Algo::Algorithm
 * (eic/juggler JugAlgo/Algorithm.h).
 *
 * Mapping from algorithm slot types to JOmniFactory handles:
 *
 *   input slot                       handle                        passed to process()
 *   -------------------------------  ----------------------------  ----------------------------
 *   C (podio collection)             PodioInput<C::value_type>     not_null<const C*>
 *   std::optional<C>                 PodioInput<..., true>         const C* (may be null)
 *   std::vector<C>                   VariadicPodioInput<...>       vector<not_null<const C*>>
 *   T (other type, e.g. Acts)        Input<T>                      not_null<const T*> (front())
 *   std::optional<T>                 Input<T, true>                const T* (front() or null)
 *
 *   output slot                      handle                        passed to process()
 *   -------------------------------  ----------------------------  ----------------------------
 *   C or std::optional<C>            PodioOutput<C::value_type>    C* (always created)
 *   std::vector<C>                   VariadicPodioOutput<...>      vector<not_null<C*>>
 *   T* (other type, e.g. Acts)       Output<T>                     T** (&emplace_back())
 *
 * Positional InputTags/OutputTags follow the order of the algorithm's Input<>/Output<>.
 *
 * Usage:
 *
 *   class MyAlgo_factory : public JOmniAlgorithmFactory<MyAlgo_factory, MyAlgo> {
 *     ParameterRef<double> m_cut{this, "cut", config().cut};   // parameters only
 *   };
 *
 * OptionalVariadicInputs = true lets std::vector<C> input slots contain names of collections
 * that may be missing; those are dropped before calling process().
 *
 * Factories that need a non-default Configure() (e.g. extra init() arguments) may define
 * their own Configure(), using m_algo; Process() remains derived.
 */

#include <algorithms/logger.h>

#include <cstdint>
#include <memory>
#include <type_traits>

#include "algorithms/interfaces/WithPodConfig.h"
#include "extensions/jana/JOmniAlgorithmSlots.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon::detail {
/// Config type of an algorithm: AlgoT::ConfigType (from WithPodConfig), or NoConfig
template <class AlgoT, class = void> struct algorithm_config {
  using type = eicrecon::NoConfig;
};
template <class AlgoT> struct algorithm_config<AlgoT, std::void_t<typename AlgoT::ConfigType>> {
  using type = typename AlgoT::ConfigType;
};
} // namespace eicrecon::detail

template <typename FactoryT, typename AlgoT,
          typename ConfigT            = typename eicrecon::detail::algorithm_config<AlgoT>::type,
          bool OptionalVariadicInputs = false>
class JOmniAlgorithmFactory : public JOmniFactory<FactoryT, ConfigT> {
  using OmniT = JOmniFactory<FactoryT, ConfigT>;

public:
  using AlgorithmType = AlgoT;

protected:
  std::unique_ptr<AlgoT> m_algo;

private:
  eicrecon::detail::SlotHandles<OmniT, typename AlgoT::input_type::data_type, true,
                                OptionalVariadicInputs>
      m_algo_inputs{this};
  eicrecon::detail::SlotHandles<OmniT, typename AlgoT::output_type::data_type, false>
      m_algo_outputs{this};
  typename OmniT::template Service<AlgorithmsInit_service> m_algorithms_init{this};

public:
  JOmniAlgorithmFactory() {
    m_algo_inputs.reorder(this->m_inputs);
    m_algo_outputs.reorder(this->m_outputs);
  }

  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(this->logger()->level()));
    // Algorithms without WithPodConfig (no ConfigType) have nothing to configure
    if constexpr (requires { typename AlgoT::ConfigType; }) {
      m_algo->applyConfig(this->config());
    }
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process(m_algo_inputs.get(), m_algo_outputs.get());
  }
};

/// Factory for algorithms that have no parameters to expose: no factory class needed.
///   app->Add(new JOmniFactoryGeneratorT<JOmniAlgorithmFactoryFor<MyAlgo>>(...));
template <typename AlgoT>
class JOmniAlgorithmFactoryFor
    : public JOmniAlgorithmFactory<JOmniAlgorithmFactoryFor<AlgoT>, AlgoT> {};
