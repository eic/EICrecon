#pragma once
#include "algorithms/event_identification/TowerTriggerConfig.h"
#include <algorithms/event_identification/TowerTrigger.h>
#include <extensions/jana/JOmniFactory.h>
#include <algorithms/event_identification/TriggerDecision.h>

namespace eicrecon {

class TowerTrigger_factory : public JOmniFactory<TowerTrigger_factory, TowerTriggerConfig> {

public:
  using AlgoT = eicrecon::TowerTrigger;

private:
  std::unique_ptr<AlgoT> m_algo;

  // TODO: Move all ParameterRefs from TimeframeSplitter into here
  PodioInput<edm4hep::EventHeader, true> m_eventHeader_inCol{this, "EventHeader"};

  VariadicPodioInput<edm4eic::TrackerHit, true> m_trackerHits_inCols{ this, {"Coll1", "Coll2"}};

  Output<TriggerDecision> m_decisions_outCol{this, "TriggerDecision"};


public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {

    try {
      // Prepare the input tuple
      // NOTE: algorithms::Algorithm currently has no support for vector<optional<T>>. 
      //       So we pretend these are vector<T> and repack inputs as vector<gsl::not_null<T*>>.

    std::vector<gsl::not_null<const edm4eic::TrackerHitCollection*>> hits;
    std::copy(m_trackerHits_inCols().cbegin(), m_trackerHits_inCols().cend(), std::back_inserter(hits));
    auto input = std::make_tuple( m_eventHeader_inCol(), hits);

    auto output_collection = TriggerDecisionCollection{.contents = m_decisions_outCol()}; 
    // TODO: I'm worried this needs a reference wrapper, otherwise the data never makes it back into m_decisions_outCol.

    auto output = std::make_tuple(&output_collection);

    m_algo->process(input, output);

    } catch (std::exception& e) {
      throw JException(e.what());
    }
  }

};


} // namespace eicrecon
