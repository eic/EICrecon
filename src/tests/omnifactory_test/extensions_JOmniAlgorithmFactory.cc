// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/JEvent.h>
#include <JANA/Utils/JTypeInfo.h>
#include <algorithms/algorithm.h>
#include <catch2/catch_test_macros.hpp>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <gsl/pointers>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

// Only the handle-generation layer is tested here; JOmniAlgorithmFactory.h also pulls in
// AlgorithmsInit_service (DD4hep, Acts), which is out of scope for this unit test.
#include "extensions/jana/JOmniAlgorithmSlots.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/log/Log_service.h"

namespace {

struct Payload {
  int value = 0;
};

using ToyInput =
    algorithms::Input<edm4hep::MCParticleCollection,
                      std::optional<edm4hep::SimTrackerHitCollection>,
                      std::vector<edm4hep::MCParticleCollection>, Payload, std::optional<Payload>>;
using ToyOutput = algorithms::Output<edm4hep::MCParticleCollection,
                                     std::optional<edm4hep::SimTrackerHitCollection>,
                                     std::vector<edm4hep::MCParticleCollection>, Payload*>;

template <bool OptionalVariadic>
class ToyFactory : public JOmniFactory<ToyFactory<OptionalVariadic>> {
  using OmniT = JOmniFactory<ToyFactory<OptionalVariadic>>;

public:
  eicrecon::detail::SlotHandles<OmniT, ToyInput::data_type, true, OptionalVariadic> in{this};
  eicrecon::detail::SlotHandles<OmniT, ToyOutput::data_type, false> out{this};

  ToyFactory() {
    in.reorder(this->m_inputs);
    out.reorder(this->m_outputs);
  }
  void Configure() {}

  // Mimic JOmniFactory::Process up to the call into the algorithm
  void Fetch(const JEvent& event) {
    for (auto* input : this->m_inputs) {
      input->GetCollection(event);
    }
    for (auto* output : this->m_outputs) {
      output->Reset();
    }
  }
};

std::shared_ptr<JEvent> make_event(JApplication& app) {
  app.ProvideService(std::make_shared<Log_service>(&app));
  auto event = std::make_shared<JEvent>(&app);
  edm4hep::MCParticleCollection a;
  a.create();
  edm4hep::MCParticleCollection b;
  b.create();
  b.create();
  event->InsertCollection<edm4hep::MCParticle>(std::move(a), "A");
  event->InsertCollection<edm4hep::MCParticle>(std::move(b), "B");
  event->Insert(new Payload{7}, "P");
  return event;
}

} // namespace

TEST_CASE("JOmniAlgorithmFactory registers handles in algorithm slot order",
          "[JOmniAlgorithmFactory]") {
  ToyFactory<false> fac;

  REQUIRE(fac.m_inputs.size() == 5);
  CHECK(fac.m_inputs[0]->type_name == JTypeInfo::demangle<edm4hep::MCParticle>());
  CHECK(fac.m_inputs[1]->type_name == JTypeInfo::demangle<edm4hep::SimTrackerHit>());
  CHECK(fac.m_inputs[2]->type_name == JTypeInfo::demangle<edm4hep::MCParticle>());
  CHECK(fac.m_inputs[3]->type_name == JTypeInfo::demangle<Payload>());
  CHECK(fac.m_inputs[4]->type_name == JTypeInfo::demangle<Payload>());
  CHECK_FALSE(fac.m_inputs[1]->is_variadic);
  CHECK(fac.m_inputs[2]->is_variadic);

  REQUIRE(fac.m_outputs.size() == 4);
  CHECK(fac.m_outputs[0]->type_name == JTypeInfo::demangle<edm4hep::MCParticle>());
  CHECK(fac.m_outputs[1]->type_name == JTypeInfo::demangle<edm4hep::SimTrackerHit>());
  CHECK(fac.m_outputs[2]->is_variadic);
  CHECK(fac.m_outputs[3]->type_name == JTypeInfo::demangle<Payload>());
}

TEST_CASE("JOmniAlgorithmFactory builds process() arguments from the event",
          "[JOmniAlgorithmFactory]") {
  JApplication app;
  auto event = make_event(app);

  ToyFactory<false> fac;
  fac.SetApplication(&app);
  // inputs: A, missing optional H, variadic {A, B}, P, missing optional Q
  fac.PreInit("toy", {"A", "H", "A", "B", "P", "Q"}, {"out", "outHits", "v1", "v2", "payload"});
  fac.Fetch(*event);

  auto [mc, hits, mcs, payload, payload_opt] = fac.in.get();
  CHECK(mc->size() == 1);
  CHECK(hits == nullptr);
  REQUIRE(mcs.size() == 2);
  CHECK(mcs[0]->size() == 1);
  CHECK(mcs[1]->size() == 2);
  CHECK(payload->value == 7);
  CHECK(payload_opt == nullptr);

  auto [out, out_hits, out_v, out_payload] = fac.out.get();
  CHECK(out->empty());
  CHECK(out_hits != nullptr);
  CHECK(out_v.size() == 2);
  CHECK(*out_payload == nullptr); // pointer slot for the algorithm to fill
}

TEST_CASE("JOmniAlgorithmFactory drops missing variadic inputs when optional",
          "[JOmniAlgorithmFactory]") {
  JApplication app;
  auto event = make_event(app);

  ToyFactory<true> fac;
  fac.SetApplication(&app);
  // variadic {B, X, A} with X missing
  fac.PreInit("toy", {"A", "H", "B", "X", "A", "P", "Q"}, {"out", "outHits", "v1", "payload"});
  fac.Fetch(*event);

  auto [mc, hits, mcs, payload, payload_opt] = fac.in.get();
  REQUIRE(mcs.size() == 2);
  CHECK(mcs[0]->size() == 2);
  CHECK(mcs[1]->size() == 1);
}
