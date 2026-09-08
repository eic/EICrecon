// SPDX-License-Identifier: LGPL-3.0-or-later

#include <catch2/catch_test_macros.hpp>
#include <edm4hep/MCParticleCollection.h>
#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <memory>
#include <string>
#include <vector>

#include "extensions/jana/JOmniFactory.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"

namespace {
class PluginNameTestFactory : public JOmniFactory<PluginNameTestFactory> {
public:
  PodioOutput<edm4hep::MCParticle> m_out{this};

  void Configure() {}
};
} // namespace

TEST_CASE("Helper factories inherit their parent's plugin name", "[JOmniFactoryGeneratorT]") {
  JApplication app;
  auto event = std::make_shared<JEvent>(&app);

  JOmniFactoryGeneratorT<PluginNameTestFactory> gen("test_tag", {}, {"TestMCParticles"}, &app);
  gen.SetPluginName("test_plugin");
  gen.GenerateFactories(event->GetFactorySet());

  bool found_helper = false;
  for (auto* factory : event->GetFactorySet()->GetAllFactories()) {
    CAPTURE(factory->GetFactoryName());
    CHECK(factory->GetPluginName() == "test_plugin");
    if (factory->GetFactoryName().find("::Helper<") != std::string::npos) {
      found_helper = true;
    }
  }
  REQUIRE(found_helper);
}
