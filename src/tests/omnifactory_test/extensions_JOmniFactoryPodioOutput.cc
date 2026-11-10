// SPDX-License-Identifier: LGPL-3.0-or-later

#include <catch2/catch_test_macros.hpp>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <string>

#include "extensions/jana/JOmniFactory.h"

namespace {

class DummyOutputFactory : public JOmniFactory<DummyOutputFactory> {
public:
  PodioOutput<edm4hep::SimCalorimeterHit> m_required_output{this, "required_hits"};
  PodioOutput<edm4hep::SimCalorimeterHit, true> m_optional_output{this, "optional_hits"};
  VariadicPodioOutput<edm4hep::SimCalorimeterHit> m_required_variadic{this, {"req1", "req2"}};
  VariadicPodioOutput<edm4hep::SimCalorimeterHit, true> m_optional_variadic{this, {"opt1", "opt2"}};

  void Configure() {}

  DummyOutputFactory() {
    for (auto* out : m_outputs) {
      out->CreateHelperFactory(*this);
    }
  }

  void TestSetCollection(OutputBase* output) { output->SetCollection(*this); }

  void TestReset(OutputBase* output) { output->Reset(); }
};

} // namespace

TEST_CASE("PodioOutput collection operators and reset", "[JOmniFactory][PodioOutput]") {
  DummyOutputFactory factory;

  // Reset allocates a new collection
  factory.TestReset(&factory.m_required_output);
  REQUIRE(factory.m_required_output() != nullptr);

  // Collection access via operator()()
  factory.m_required_output()->create();
  REQUIRE(factory.m_required_output()->size() == 1);

  // Clearing the collection via std::unique_ptr::reset()
  factory.m_required_output().reset();
  REQUIRE(factory.m_required_output() == nullptr);
}

TEST_CASE("PodioOutput SetCollection optional vs required behavior",
          "[JOmniFactory][PodioOutput]") {
  DummyOutputFactory factory;

  SECTION("Required output throws JException when m_data is null") {
    factory.m_required_output().reset();
    REQUIRE_THROWS_AS(factory.TestSetCollection(&factory.m_required_output), JException);
  }

  SECTION("Optional output does not throw when m_data is null") {
    factory.m_optional_output().reset();
    REQUIRE_NOTHROW(factory.TestSetCollection(&factory.m_optional_output));
  }

  SECTION("Required output succeeds when data is present") {
    factory.TestReset(&factory.m_required_output);
    REQUIRE_NOTHROW(factory.TestSetCollection(&factory.m_required_output));
  }

  SECTION("Optional output succeeds when data is present") {
    factory.TestReset(&factory.m_optional_output);
    REQUIRE_NOTHROW(factory.TestSetCollection(&factory.m_optional_output));
  }
}

TEST_CASE("VariadicPodioOutput SetCollection optional vs required behavior",
          "[JOmniFactory][VariadicPodioOutput]") {
  DummyOutputFactory factory;

  SECTION("Required variadic output throws when collection size mismatches") {
    factory.m_required_variadic().clear();
    REQUIRE_THROWS_AS(factory.TestSetCollection(&factory.m_required_variadic), JException);
  }

  SECTION("Optional variadic output does not throw when collection size mismatches") {
    factory.m_optional_variadic().clear();
    REQUIRE_NOTHROW(factory.TestSetCollection(&factory.m_optional_variadic));
  }

  SECTION("Required variadic output throws when an element is null") {
    factory.TestReset(&factory.m_required_variadic);
    factory.m_required_variadic()[0] = nullptr;
    REQUIRE_THROWS_AS(factory.TestSetCollection(&factory.m_required_variadic), JException);
  }

  SECTION("Optional variadic output does not throw when an element is null") {
    factory.TestReset(&factory.m_optional_variadic);
    factory.m_optional_variadic()[0] = nullptr;
    REQUIRE_NOTHROW(factory.TestSetCollection(&factory.m_optional_variadic));
  }

  SECTION("Optional variadic output does not throw when all elements are null") {
    factory.TestReset(&factory.m_optional_variadic);
    factory.m_optional_variadic()[0] = nullptr;
    factory.m_optional_variadic()[1] = nullptr;
    REQUIRE_NOTHROW(factory.TestSetCollection(&factory.m_optional_variadic));
  }

  SECTION("Required variadic output succeeds when all collections are present") {
    factory.TestReset(&factory.m_required_variadic);
    REQUIRE_NOTHROW(factory.TestSetCollection(&factory.m_required_variadic));
  }

  SECTION("Optional variadic output succeeds when all collections are present") {
    factory.TestReset(&factory.m_optional_variadic);
    REQUIRE_NOTHROW(factory.TestSetCollection(&factory.m_optional_variadic));
  }
}
