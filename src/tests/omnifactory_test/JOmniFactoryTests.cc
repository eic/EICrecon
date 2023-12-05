
#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/JFactorySet.h>
#include <JANA/JMultifactory.h>
#include <JANA/Services/JComponentManager.h>
#include <JANA/Services/JParameterManager.h>
#include <catch2/catch_test_macros.hpp>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <fmt/core.h>
#include <spdlog/logger.h>
#include <stdint.h>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "extensions/jana/JOmniFactory.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"

struct BasicTestAlgConfig {
    int bucket_count = 42;
    double threshold = 7.6;
};

struct BasicTestAlg : public JOmniFactory<BasicTestAlg, BasicTestAlgConfig> {

    PodioOutput<edm4hep::SimCalorimeterHit> output_hits_left {this, "output_hits_left"};
    PodioOutput<edm4hep::SimCalorimeterHit> output_hits_right {this, "output_hits_right"};
    Output<edm4hep::SimCalorimeterHit> output_vechits {this, "output_vechits"};

    ParameterRef<int> bucket_count {this, "bucket_count", config().bucket_count, "The total number of buckets [dimensionless]"};
    ParameterRef<double> threshold {this, "threshold", config().threshold, "The max cutoff threshold [V * A * kg^-1 * m^-2 * sec^-3]"};

    std::vector<OutputBase*> GetOutputs() { return this->m_outputs; }

    int m_init_call_count = 0;
    int m_changerun_call_count = 0;
    int m_process_call_count = 0;

    void Configure() {
        m_init_call_count++;
        logger()->info("Calling BasicTestAlg::Configure");
    }

    void ChangeRun(int64_t run_number) {
        m_changerun_call_count++;
        logger()->info("Calling BasicTestAlg::ChangeRun");
    }

    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    void Process(int64_t run_number, uint64_t event_number) {
        m_process_call_count++;
        logger()->info("Calling BasicTestAlg::Process with bucket_count={}, threshold={}", config().bucket_count, config().threshold);
        // Provide empty collections (as opposed to nulls) so that PODIO doesn't crash
        // TODO: NWB: I though multifactories already took care of this under the hood somewhere
        output_hits_left() = std::make_unique<edm4hep::SimCalorimeterHitCollection>();
        output_hits_right() = std::make_unique<edm4hep::SimCalorimeterHitCollection>();
        output_vechits().push_back(new edm4hep::SimCalorimeterHit());
    }
};

template <typename OutputCollectionT, typename MultifactoryT>
MultifactoryT* RetrieveMultifactory(JFactorySet* facset, std::string output_collection_name) {
    auto fac = facset->GetFactory<OutputCollectionT>(output_collection_name);
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
    REQUIRE(alg.GetOutputs()[0]->collection_name == "output_hits_left");
    REQUIRE(alg.GetOutputs()[0]->type_name == "edm4hep::SimCalorimeterHit");
    REQUIRE(alg.GetOutputs()[1]->collection_name == "output_hits_right");
    REQUIRE(alg.GetOutputs()[1]->type_name == "edm4hep::SimCalorimeterHit");
    REQUIRE(alg.GetOutputs()[2]->collection_name == "output_vechits");
    REQUIRE(alg.GetOutputs()[2]->type_name == "edm4hep::SimCalorimeterHit");
}

TEST_CASE("Configuration object is correctly wired from untyped wiring data") {
    JApplication app;
    app.AddPlugin("log");
    app.Initialize();
    JOmniFactoryGeneratorT<BasicTestAlg> facgen (&app);
    facgen.AddWiring("ECalTestAlg", {}, {"ECalLeftHits", "ECalRightHits", "ECalVecHits"}, {{"threshold", "6.1"}, {"bucket_count", "22"}});

    JFactorySet facset;
    facgen.GenerateFactories(&facset);
    // for (auto* fac : facset.GetAllFactories()) {
        // std::cout << "typename=" << fac->GetFactoryName() << ", tag=" << fac->GetTag() << std::endl;
    // }

    auto basictestalg = RetrieveMultifactory<edm4hep::SimCalorimeterHit,BasicTestAlg>(&facset, "ECalLeftHits");

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
    JOmniFactoryGeneratorT<BasicTestAlg> facgen (&app);
    facgen.AddWiring("BCalTestAlg", {}, {"BCalLeftHits", "BCalRightHits", "BCalVecHits"}, {{"threshold", "6.1"}, {"bucket_count", "22"}});
    facgen.AddWiring("CCalTestAlg", {}, {"CCalLeftHits", "CCalRightHits", "CCalVecHits"}, {{"threshold", "9.0"}, {"bucket_count", "27"}});
    facgen.AddWiring("ECalTestAlg", {}, {"ECalLeftHits", "ECalRightHits", "ECalVecHits"}, {{"threshold", "16.25"}, {"bucket_count", "49"}});

    JFactorySet facset;
    facgen.GenerateFactories(&facset);
    // for (auto* fac : facset.GetAllFactories()) {
        // std::cout << "typename=" << fac->GetFactoryName() << ", tag=" << fac->GetTag() << std::endl;
    // }
    auto b = RetrieveMultifactory<edm4hep::SimCalorimeterHit,BasicTestAlg>(&facset, "BCalLeftHits");
    auto c = RetrieveMultifactory<edm4hep::SimCalorimeterHit,BasicTestAlg>(&facset, "CCalLeftHits");
    auto e = RetrieveMultifactory<edm4hep::SimCalorimeterHit,BasicTestAlg>(&facset, "ECalLeftHits");

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

TEST_CASE("JParameterManager correctly understands which values are defaulted and which are overridden") {
    JApplication app;
    app.AddPlugin("log");

    auto facgen = new JOmniFactoryGeneratorT<BasicTestAlg>(&app);
    facgen->AddWiring("FunTest", {}, {"BCalLeftHits", "BCalRightHits", "BCalVecHits"}, {{"threshold", "6.1"}, {"bucket_count", "22"}});
    app.Add(facgen);

    app.GetJParameterManager()->SetParameter("FunTest:threshold", 12.0);
    app.Initialize();

    auto event = std::make_shared<JEvent>();
    app.GetService<JComponentManager>()->configure_event(*event);

    // for (auto* fac : event->GetFactorySet()->GetAllFactories()) {
        // std::cout << "typename=" << fac->GetFactoryName() << ", tag=" << fac->GetTag() << std::endl;
    // }

    // Retrieve multifactory
    auto b = RetrieveMultifactory<edm4hep::SimCalorimeterHit,BasicTestAlg>(event->GetFactorySet(), "BCalLeftHits");

    // Overrides won't happen until factory gets Init()ed. However, defaults will be applied immediately
    REQUIRE(b->threshold() == 6.1);
    REQUIRE(b->config().threshold == 6.1);

    // Trigger JMF::Execute(), in order to trigger Init(), in order to Configure()s all Parameter fields...
    auto lefthits = event->Get<edm4hep::SimCalorimeterHit>("BCalLeftHits");

    REQUIRE(b->threshold() == 12.0);
    REQUIRE(b->config().threshold == 12.0);

    std::cout << "Showing the full table of config parameters" << std::endl;
    app.GetJParameterManager()->PrintParameters(true, false, true);

    std::cout << "Showing only overridden config parameters" << std::endl;
    app.GetJParameterManager()->PrintParameters(false, false, true);
}

TEST_CASE("Wiring itself is correctly defaulted") {
    JApplication app;
    app.AddPlugin("log");

    auto facgen = new JOmniFactoryGeneratorT<BasicTestAlg>(&app);
    facgen->AddWiring("FunTest", {}, {"BCalLeftHits", "BCalRightHits", "BCalVecHits"}, {{"threshold", "6.1"}});
    app.Add(facgen);
    app.Initialize();

    auto event = std::make_shared<JEvent>();
    app.GetService<JComponentManager>()->configure_event(*event);

    // Retrieve multifactory
    auto b = RetrieveMultifactory<edm4hep::SimCalorimeterHit,BasicTestAlg>(event->GetFactorySet(), "BCalLeftHits");

    // Overrides won't happen until factory gets Init()ed. However, defaults will be applied immediately
    REQUIRE(b->bucket_count() == 42);      // Not provided by wiring
    REQUIRE(b->config().bucket_count == 42);  // Not provided by wiring

    REQUIRE(b->threshold() == 6.1);        // Provided by wiring
    REQUIRE(b->config().threshold == 6.1);    // Provided by wiring

    // Trigger JMF::Execute(), in order to trigger Init(), in order to Configure()s all Parameter fields...
    auto lefthits = event->Get<edm4hep::SimCalorimeterHit>("BCalLeftHits");

    // We didn't override the config values via the parameter manager, so all of these should be the same
    REQUIRE(b->bucket_count() == 42);      // Not provided by wiring
    REQUIRE(b->config().bucket_count == 42);  // Not provided by wiring

    REQUIRE(b->threshold() == 6.1);        // Provided by wiring
    REQUIRE(b->config().threshold == 6.1);    // Provided by wiring


    b->logger()->info("Showing the full table of config parameters");
    app.GetJParameterManager()->PrintParameters(true, false, true);

    b->logger()->info("Showing only overridden config parameters");
    // Should be empty because everything is defaulted
    app.GetJParameterManager()->PrintParameters(false, false, true);
}
