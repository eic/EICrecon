
#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/JEvent.h>
#include <JANA/JFactorySet.h>
#include <JANA/JMultifactory.h>
#include <JANA/Services/JComponentManager.h>
#include <JANA/Services/JParameterManager.h>
#include <JANA/Utils/JTypeInfo.h>
#include <catch2/catch_test_macros.hpp>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <spdlog/logger.h>
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "extensions/jana/JOmniFactory.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"

struct BasicTestAlgConfig {
  int bucket_count = 42;
  double threshold = 7.6;
};

struct BasicTestAlg : public JOmniFactory<BasicTestAlg, BasicTestAlgConfig> {

  PodioOutput<edm4hep::SimCalorimeterHit> output_hits_left{this, "output_hits_left"};
  PodioOutput<edm4hep::SimCalorimeterHit> output_hits_right{this, "output_hits_right"};
  Output<edm4hep::SimCalorimeterHit> output_vechits{this, "output_vechits"};

  ParameterRef<int> bucket_count{this, "bucket_count", config().bucket_count,
                                 "The total number of buckets [dimensionless]"};
  ParameterRef<double> threshold{this, "threshold", config().threshold,
                                 "The max cutoff threshold [V * A * kg^-1 * m^-2 * sec^-3]"};

  std::vector<OutputBase*> GetOutputs() { return this->m_outputs; }

  int m_init_call_count      = 0;
  int m_changerun_call_count = 0;
  int m_process_call_count   = 0;

  void Configure() {
    m_init_call_count++;
    logger()->info("Calling BasicTestAlg::Configure");
  }

  void ChangeRun(int32_t /* run_number */) override {
    m_changerun_call_count++;
    logger()->info("Calling BasicTestAlg::ChangeRun");
  }

  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  void Process(int32_t /* run_number */, uint64_t /* event_number */) override {
    m_process_call_count++;
    logger()->info("Calling BasicTestAlg::Process with bucket_count={}, threshold={}",
                   config().bucket_count, config().threshold);
    // Provide empty collections (as opposed to nulls) so that PODIO doesn't crash
    output_hits_left()  = std::make_unique<edm4hep::SimCalorimeterHitCollection>();
    output_hits_right() = std::make_unique<edm4hep::SimCalorimeterHitCollection>();
    output_vechits().push_back(new edm4hep::SimCalorimeterHit());
  }
};

template <typename OutputCollectionT, typename MultifactoryT>
MultifactoryT* RetrieveMultifactory(JFactorySet* facset, std::string output_collection_name) {
  auto fac = facset->GetFactory(JTypeInfo::demangle<OutputCollectionT>(), output_collection_name);
  REQUIRE(fac != nullptr);
  auto helper = dynamic_cast<JMultifactoryHelperPodio<OutputCollectionT>*>(fac);
  REQUIRE(helper != nullptr);
  auto multifactory = helper->GetMultifactory();
  REQUIRE(multifactory != nullptr);
  auto typed = dynamic_cast<MultifactoryT*>(multifactory);
  REQUIRE(typed != nullptr);
  return typed;
}

TEST_CASE("Registering Podio outputs works") {
  BasicTestAlg alg;
  REQUIRE(alg.GetOutputs().size() == 3);
  REQUIRE(alg.GetOutputs()[0]->collection_names[0] == "output_hits_left");
  REQUIRE(alg.GetOutputs()[0]->type_name == "edm4hep::SimCalorimeterHit");
  REQUIRE(alg.GetOutputs()[1]->collection_names[0] == "output_hits_right");
  REQUIRE(alg.GetOutputs()[1]->type_name == "edm4hep::SimCalorimeterHit");
  REQUIRE(alg.GetOutputs()[2]->collection_names[0] == "output_vechits");
  REQUIRE(alg.GetOutputs()[2]->type_name == "edm4hep::SimCalorimeterHit");
}

TEST_CASE("Configuration object is correctly wired from untyped wiring data") {
  JApplication app;
  app.AddPlugin("log");
  app.Initialize();
  JOmniFactoryGeneratorT<BasicTestAlg> facgen(&app);
  facgen.AddWiring("ECalTestAlg", {}, {"ECalLeftHits", "ECalRightHits", "ECalVecHits"},
                   {{"threshold", "6.1"}, {"bucket_count", "22"}});

  JFactorySet facset;
  facgen.GenerateFactories(&facset);
  // for (auto* fac : facset.GetAllFactories()) {
  // std::cout << "typename=" << fac->GetFactoryName() << ", tag=" << fac->GetTag() << std::endl;
  // }

  auto* basictestalg =
      RetrieveMultifactory<edm4hep::SimCalorimeterHit, BasicTestAlg>(&facset, "ECalLeftHits");

  REQUIRE(basictestalg->threshold() == 6.1);
  REQUIRE(basictestalg->bucket_count() == 22);

  REQUIRE(basictestalg->config().threshold == 6.1);
  REQUIRE(basictestalg->config().bucket_count == 22);

  REQUIRE(basictestalg->m_init_call_count == 0);
}

TEST_CASE("Multiple configuration objects are correctly wired from untyped wiring data") {
  JApplication app;
  app.AddPlugin("log");
  app.Initialize();
  JOmniFactoryGeneratorT<BasicTestAlg> facgen(&app);
  facgen.AddWiring("BCalTestAlg", {}, {"BCalLeftHits", "BCalRightHits", "BCalVecHits"},
                   {{"threshold", "6.1"}, {"bucket_count", "22"}});
  facgen.AddWiring("CCalTestAlg", {}, {"CCalLeftHits", "CCalRightHits", "CCalVecHits"},
                   {{"threshold", "9.0"}, {"bucket_count", "27"}});
  facgen.AddWiring("ECalTestAlg", {}, {"ECalLeftHits", "ECalRightHits", "ECalVecHits"},
                   {{"threshold", "16.25"}, {"bucket_count", "49"}});

  JFactorySet facset;
  facgen.GenerateFactories(&facset);
  // for (auto* fac : facset.GetAllFactories()) {
  // std::cout << "typename=" << fac->GetFactoryName() << ", tag=" << fac->GetTag() << std::endl;
  // }
  auto* b = RetrieveMultifactory<edm4hep::SimCalorimeterHit, BasicTestAlg>(&facset, "BCalLeftHits");
  auto* c = RetrieveMultifactory<edm4hep::SimCalorimeterHit, BasicTestAlg>(&facset, "CCalLeftHits");
  auto* e = RetrieveMultifactory<edm4hep::SimCalorimeterHit, BasicTestAlg>(&facset, "ECalLeftHits");

  REQUIRE(b->threshold() == 6.1);
  REQUIRE(b->bucket_count() == 22);
  REQUIRE(b->config().threshold == 6.1);
  REQUIRE(b->config().bucket_count == 22);

  REQUIRE(c->threshold() == 9.0);
  REQUIRE(c->bucket_count() == 27);
  REQUIRE(c->config().threshold == 9.0);
  REQUIRE(c->config().bucket_count == 27);

  REQUIRE(e->threshold() == 16.25);
  REQUIRE(e->bucket_count() == 49);
  REQUIRE(e->config().threshold == 16.25);
  REQUIRE(e->config().bucket_count == 49);

  REQUIRE(b->m_init_call_count == 0);
  REQUIRE(c->m_init_call_count == 0);
  REQUIRE(e->m_init_call_count == 0);
}

TEST_CASE(
    "JParameterManager correctly understands which values are defaulted and which are overridden") {
  JApplication app;
  app.AddPlugin("log");

  auto* facgen = new JOmniFactoryGeneratorT<BasicTestAlg>(&app);
  facgen->AddWiring("FunTest", {}, {"BCalLeftHits", "BCalRightHits", "BCalVecHits"},
                    {{"threshold", "6.1"}, {"bucket_count", "22"}});
  app.Add(facgen);

  app.GetJParameterManager()->SetParameter("FunTest:threshold", 12.0);
  app.Initialize();

  auto event = std::make_shared<JEvent>();
  app.GetService<JComponentManager>()->configure_event(*event);

  // for (auto* fac : event->GetFactorySet()->GetAllFactories()) {
  // std::cout << "typename=" << fac->GetFactoryName() << ", tag=" << fac->GetTag() << std::endl;
  // }

  // Retrieve multifactory
  auto* b = RetrieveMultifactory<edm4hep::SimCalorimeterHit, BasicTestAlg>(event->GetFactorySet(),
                                                                           "BCalLeftHits");

  // Overrides won't happen until factory gets Init()ed. However, defaults will be applied immediately
  REQUIRE(b->threshold() == 6.1);
  REQUIRE(b->config().threshold == 6.1);

  // Trigger JMF::Execute(), in order to trigger Init(), in order to Configure()s all Parameter fields...
  auto* lefthits = event->GetCollection<edm4hep::SimCalorimeterHit>("BCalLeftHits");

  REQUIRE(b->threshold() == 12.0);
  REQUIRE(b->config().threshold == 12.0);

  std::cout << "Showing the full table of config parameters" << std::endl;
  app.GetJParameterManager()->PrintParameters(2, 1); // verbosity, strictness

  std::cout << "Showing only overridden config parameters" << std::endl;
  app.GetJParameterManager()->PrintParameters(1, 1); // verbosity, strictness
}

TEST_CASE("Wiring itself is correctly defaulted") {
  JApplication app;
  app.AddPlugin("log");

  auto* facgen = new JOmniFactoryGeneratorT<BasicTestAlg>(&app);
  facgen->AddWiring("FunTest", {}, {"BCalLeftHits", "BCalRightHits", "BCalVecHits"},
                    {{"threshold", "6.1"}});
  app.Add(facgen);
  app.Initialize();

  auto event = std::make_shared<JEvent>();
  app.GetService<JComponentManager>()->configure_event(*event);

  // Retrieve multifactory
  auto* b = RetrieveMultifactory<edm4hep::SimCalorimeterHit, BasicTestAlg>(event->GetFactorySet(),
                                                                           "BCalLeftHits");

  // Overrides won't happen until factory gets Init()ed. However, defaults will be applied immediately
  REQUIRE(b->bucket_count() == 42);        // Not provided by wiring
  REQUIRE(b->config().bucket_count == 42); // Not provided by wiring

  REQUIRE(b->threshold() == 6.1);        // Provided by wiring
  REQUIRE(b->config().threshold == 6.1); // Provided by wiring

  // Trigger JMF::Execute(), in order to trigger Init(), in order to Configure()s all Parameter fields...
  auto* lefthits = event->GetCollection<edm4hep::SimCalorimeterHit>("BCalLeftHits");

  // We didn't override the config values via the parameter manager, so all of these should be the same
  REQUIRE(b->bucket_count() == 42);        // Not provided by wiring
  REQUIRE(b->config().bucket_count == 42); // Not provided by wiring

  REQUIRE(b->threshold() == 6.1);        // Provided by wiring
  REQUIRE(b->config().threshold == 6.1); // Provided by wiring

  b->logger()->info("Showing the full table of config parameters");
  app.GetJParameterManager()->PrintParameters(2, 1); // verbosity, strictness

  b->logger()->info("Showing only overridden config parameters");
  // Should be empty because everything is defaulted
  app.GetJParameterManager()->PrintParameters(1, 1); // verbosity, strictness
}

struct VariadicTestAlg : public JOmniFactory<VariadicTestAlg, BasicTestAlgConfig> {

  PodioInput<edm4hep::SimCalorimeterHit> m_hits_in{this};
  VariadicPodioInput<edm4hep::SimCalorimeterHit> m_variadic_hits_in{this};
  PodioOutput<edm4hep::SimCalorimeterHit> m_hits_out{this};

  std::vector<OutputBase*> GetOutputs() { return this->m_outputs; }

  int m_init_call_count      = 0;
  int m_changerun_call_count = 0;
  int m_process_call_count   = 0;

  void Configure() {
    m_init_call_count++;
    logger()->info("Calling VariadicTestAlg::Configure");
  }

  void ChangeRun(int32_t /* run_number */) override {
    m_changerun_call_count++;
    logger()->info("Calling VariadicTestAlg::ChangeRun");
  }

  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  void Process(int32_t /* run_number */, uint64_t /* event_number */) override {
    m_process_call_count++;
    logger()->info("Calling VariadicTestAlg::Process with bucket_count={}, threshold={}",
                   config().bucket_count, config().threshold);

    REQUIRE(m_hits_in()->size() == 3);
    REQUIRE(m_variadic_hits_in().size() == 2);
    REQUIRE(m_variadic_hits_in()[0]->size() == 1);
    REQUIRE(m_variadic_hits_in()[1]->size() == 2);

    m_hits_out() = std::make_unique<edm4hep::SimCalorimeterHitCollection>();
    m_hits_out()->create();
    m_hits_out()->create();
    m_hits_out()->create();
    m_hits_out()->create();
  }
};

TEST_CASE("VariadicOmniFactoryTests") {
  VariadicTestAlg alg;
  JApplication app;
  app.AddPlugin("log");

  auto* facgen = new JOmniFactoryGeneratorT<VariadicTestAlg>(
      "VariadicTest", {"main_hits", "fun_hits", "funner_hits"}, {"processed_hits"}, &app);
  app.Add(facgen);
  app.Initialize();

  auto event = std::make_shared<JEvent>();
  app.GetService<JComponentManager>()->configure_event(*event);

  edm4hep::SimCalorimeterHitCollection mains;
  edm4hep::SimCalorimeterHitCollection funs;
  edm4hep::SimCalorimeterHitCollection funners;

  mains.create();
  mains.create();
  mains.create();

  funs.create();
  funners.create();
  funners.create();

  event->InsertCollection<edm4hep::SimCalorimeterHit>(std::move(mains), "main_hits");
  event->InsertCollection<edm4hep::SimCalorimeterHit>(std::move(funs), "fun_hits");
  event->InsertCollection<edm4hep::SimCalorimeterHit>(std::move(funners), "funner_hits");

  const auto* processed = event->GetCollection<edm4hep::SimCalorimeterHit>("processed_hits");
  REQUIRE(processed->size() == 4);
}

struct SubsetTestAlg : public JOmniFactory<SubsetTestAlg, BasicTestAlgConfig> {

  VariadicPodioInput<edm4hep::SimCalorimeterHit> m_left_hits_in{this};
  PodioInput<edm4hep::SimCalorimeterHit> m_center_hits_in{this};
  VariadicPodioInput<edm4hep::SimCalorimeterHit> m_right_hits_in{this};
  PodioOutput<edm4hep::SimCalorimeterHit> m_hits_out{this};

  std::vector<OutputBase*> GetOutputs() { return this->m_outputs; }

  void Configure() {}

  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  void Process(int32_t /* run_number */, uint64_t /* event_number */) override {

    // Variadic collection count constrained to be same size
    REQUIRE(m_left_hits_in().size() == 1);
    REQUIRE(m_right_hits_in().size() == 1);

    REQUIRE(m_left_hits_in()[0]->size() == 2);
    REQUIRE(m_right_hits_in()[0]->size() == 1);

    REQUIRE(m_center_hits_in()->size() == 3);

    m_hits_out() = std::make_unique<edm4hep::SimCalorimeterHitCollection>();
    m_hits_out()->setSubsetCollection();

    const auto* lhi = m_left_hits_in()[0];
    for (const auto& hit : *lhi) {
      m_hits_out()->push_back(hit);
    }
    for (const auto& hit : *m_center_hits_in()) {
      m_hits_out()->push_back(hit);
    }
  }
};

TEST_CASE("SubsetOmniFactoryTests") {
  JApplication app;
  app.AddPlugin("log");

  auto* facgen = new JOmniFactoryGeneratorT<SubsetTestAlg>(
      "SubsetTest", {"left", "center", "right"}, {"processed_hits"}, &app);
  app.Add(facgen);
  app.Initialize();

  auto event = std::make_shared<JEvent>();
  app.GetService<JComponentManager>()->configure_event(*event);

  edm4hep::SimCalorimeterHitCollection left;
  edm4hep::SimCalorimeterHitCollection center;
  edm4hep::SimCalorimeterHitCollection right;

  left.create();
  left.create();
  right.create();

  center.create();
  center.create();
  center.create();

  event->InsertCollection<edm4hep::SimCalorimeterHit>(std::move(left), "left");
  event->InsertCollection<edm4hep::SimCalorimeterHit>(std::move(center), "center");
  event->InsertCollection<edm4hep::SimCalorimeterHit>(std::move(right), "right");

  const auto* processed = event->GetCollection<edm4hep::SimCalorimeterHit>("processed_hits");
  REQUIRE(processed->size() == 5);
}

struct VariadicOutputTestAlg : public JOmniFactory<VariadicOutputTestAlg, BasicTestAlgConfig> {

  PodioInput<edm4hep::SimCalorimeterHit> m_hits_in{this};

  VariadicPodioOutput<edm4hep::SimCalorimeterHit> m_hits_out{this};

  void Configure() {}

  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  void Process(int32_t /* run_number */, uint64_t /* event_number */) override {

    REQUIRE(m_hits_out().size() == 2);
    m_hits_out()[0]->setSubsetCollection();
    m_hits_out()[1]->setSubsetCollection();

    int i = 0;
    for (const auto& hit : *(m_hits_in())) {
      m_hits_out()[i]->push_back(hit);
      i = (i == 1) ? 0 : 1;
    }
  }
};

TEST_CASE("VariadicPodioOutputTests") {
  JApplication app;
  app.AddPlugin("log");

  auto* facgen = new JOmniFactoryGeneratorT<VariadicOutputTestAlg>(
      "VariadicOutputTest", {"all_hits"}, {"left_hits", "right_hits"}, &app);
  app.Add(facgen);
  app.Initialize();

  auto event = std::make_shared<JEvent>();
  app.GetService<JComponentManager>()->configure_event(*event);

  edm4hep::SimCalorimeterHitCollection all_hits;

  all_hits.create();
  all_hits.create();
  all_hits.create();

  event->InsertCollection<edm4hep::SimCalorimeterHit>(std::move(all_hits), "all_hits");

  const auto* left_hits  = event->GetCollection<edm4hep::SimCalorimeterHit>("left_hits");
  const auto* right_hits = event->GetCollection<edm4hep::SimCalorimeterHit>("right_hits");
  REQUIRE(left_hits->size() == 2);
  REQUIRE(right_hits->size() == 1);
}

struct OptionalPodioInputTestAlg
    : public JOmniFactory<OptionalPodioInputTestAlg, BasicTestAlgConfig> {

  PodioInput<edm4hep::SimCalorimeterHit, true> m_left_hits_in{this};
  PodioInput<edm4hep::SimCalorimeterHit, false> m_right_hits_in{this};

  PodioOutput<edm4hep::SimCalorimeterHit> m_left_hits_out{this};
  PodioOutput<edm4hep::SimCalorimeterHit> m_right_hits_out{this};

  void Configure() {}

  void Process(int32_t /* run_number */, uint64_t /* event_number */) override {

    logger()->info("Calling OptionalPodioInputTestAlg::Process");

    m_left_hits_out()  = std::make_unique<edm4hep::SimCalorimeterHitCollection>();
    m_right_hits_out() = std::make_unique<edm4hep::SimCalorimeterHitCollection>();
    //Set the subset flag
    m_left_hits_out()->setSubsetCollection();
    m_right_hits_out()->setSubsetCollection();

    //Copy hits to output collections
    if (m_left_hits_in() != nullptr) {
      for (const auto& hit : *(m_left_hits_in())) {
        m_left_hits_out()->push_back(hit);
      }
    }
    if (m_right_hits_in() != nullptr) {
      for (const auto& hit : *(m_right_hits_in())) {
        m_right_hits_out()->push_back(hit);
      }
    }
  }
};

TEST_CASE("Optional PodioInput") {
  JApplication app;
  app.AddPlugin("log");

  auto* facgen = new JOmniFactoryGeneratorT<OptionalPodioInputTestAlg>(
      "OptionalPodioInputTest", {"left_hits", "right_hits"}, {"left_hits_out", "right_hits_out"},
      &app);

  app.Add(facgen);
  app.Initialize();

  auto event = std::make_shared<JEvent>();
  app.GetService<JComponentManager>()->configure_event(*event);

  SECTION("Both collections are set") {
    edm4hep::SimCalorimeterHitCollection left_hits;
    edm4hep::SimCalorimeterHitCollection right_hits;
    left_hits.create();
    left_hits.create();
    right_hits.create();
    event->InsertCollection<edm4hep::SimCalorimeterHit>(std::move(left_hits), "left_hits");
    event->InsertCollection<edm4hep::SimCalorimeterHit>(std::move(right_hits), "right_hits");

    const auto* left_hits_out  = event->GetCollection<edm4hep::SimCalorimeterHit>("left_hits_out");
    const auto* right_hits_out = event->GetCollection<edm4hep::SimCalorimeterHit>("right_hits_out");
    REQUIRE(left_hits_out->size() == 2);
    REQUIRE(right_hits_out->size() == 1);
  }
  SECTION("Left hits are not set") {
    edm4hep::SimCalorimeterHitCollection right_hits;
    right_hits.create();
    event->InsertCollection<edm4hep::SimCalorimeterHit>(std::move(right_hits), "right_hits");

    const auto* left_hits_out  = event->GetCollection<edm4hep::SimCalorimeterHit>("left_hits_out");
    const auto* right_hits_out = event->GetCollection<edm4hep::SimCalorimeterHit>("right_hits_out");
    REQUIRE(left_hits_out->empty());
    REQUIRE(right_hits_out->size() == 1);
  }
  SECTION("Right hits are not set") {
    edm4hep::SimCalorimeterHitCollection left_hits;
    left_hits.create();
    left_hits.create();
    event->InsertCollection<edm4hep::SimCalorimeterHit>(std::move(left_hits), "left_hits");

    REQUIRE_THROWS(event->GetCollection<edm4hep::SimCalorimeterHit>("left_hits_out"));
    REQUIRE_THROWS(event->GetCollection<edm4hep::SimCalorimeterHit>("right_hits_out"));
  }
}

struct OptionalVariadicPodioInputTestAlg
    : public JOmniFactory<OptionalVariadicPodioInputTestAlg, BasicTestAlgConfig> {

  VariadicPodioInput<edm4hep::SimCalorimeterHit, true> m_hits_in{this};

  PodioOutput<edm4hep::SimCalorimeterHit> m_hits_out{this};

  void Configure() {}

  void Process(int32_t /* run_number */, uint64_t /* event_number */) override {

    logger()->info("Calling OptionalVariadicPodioInputTestAlg::Process");

    m_hits_out() = std::make_unique<edm4hep::SimCalorimeterHitCollection>();
    //Set the subset flag
    m_hits_out()->setSubsetCollection();
    //Copy hits to output collections
    for (const auto& coll : m_hits_in()) {
      if (coll != nullptr) {
        for (const auto& hit : *coll) {
          m_hits_out()->push_back(hit);
        }
      }
    }
  }
};

TEST_CASE("Optional Variadic Podio Input") {
  JApplication app;
  app.AddPlugin("log");

  auto* facgen = new JOmniFactoryGeneratorT<OptionalVariadicPodioInputTestAlg>(
      "OptionalVariadicPodioInputTest", {"left_hits", "center_hits", "right_hits"}, {"hits_out"},
      &app);

  app.Add(facgen);
  app.Initialize();

  auto event = std::make_shared<JEvent>();
  app.GetService<JComponentManager>()->configure_event(*event);

  SECTION("All collections are set") {
    edm4hep::SimCalorimeterHitCollection left_hits;
    edm4hep::SimCalorimeterHitCollection center_hits;
    edm4hep::SimCalorimeterHitCollection right_hits;
    left_hits.create();
    left_hits.create();
    left_hits.create();
    center_hits.create();
    center_hits.create();
    right_hits.create();
    event->InsertCollection<edm4hep::SimCalorimeterHit>(std::move(left_hits), "left_hits");
    event->InsertCollection<edm4hep::SimCalorimeterHit>(std::move(center_hits), "center_hits");
    event->InsertCollection<edm4hep::SimCalorimeterHit>(std::move(right_hits), "right_hits");

    const auto* hits_out = event->GetCollection<edm4hep::SimCalorimeterHit>("hits_out");

    REQUIRE(hits_out->size() == 6);
  }

  SECTION("No collections are set") {
    const auto* hits_out = event->GetCollection<edm4hep::SimCalorimeterHit>("hits_out");

    REQUIRE(hits_out->empty());
  }
  SECTION("Only right collection is set") {
    edm4hep::SimCalorimeterHitCollection right_hits;
    right_hits.create();
    event->InsertCollection<edm4hep::SimCalorimeterHit>(std::move(right_hits), "right_hits");

    const auto* hits_out = event->GetCollection<edm4hep::SimCalorimeterHit>("hits_out");
    REQUIRE(hits_out->size() == 1);
  }
}
