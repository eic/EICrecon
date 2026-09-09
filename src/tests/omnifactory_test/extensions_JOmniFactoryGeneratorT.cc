// SPDX-License-Identifier: LGPL-3.0-or-later

#include <catch2/catch_test_macros.hpp>
#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/Services/JParameterManager.h>
#include <memory>

#include "extensions/jana/JOmniFactory.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "services/log/Log_service.h"

namespace {
class PluginNameTestFactory : public JOmniFactory<PluginNameTestFactory> {
public:
  void Configure() {}
};
} // namespace

TEST_CASE("SetPluginName before PreInit makes the plugin name part of the registered "
          "parameter prefix",
          "[JOmniFactoryGeneratorT]") {
  JApplication app;
  app.ProvideService(std::make_shared<Log_service>(&app));
  auto event = std::make_shared<JEvent>(&app);

  JOmniFactoryGeneratorT<PluginNameTestFactory> gen("test_tag", {}, {}, &app);
  gen.SetPluginName("test_plugin");
  gen.GenerateFactories(event->GetFactorySet());

  // PreInit() computes m_prefix from GetPluginName() and uses it to register
  // "<prefix>:InputTags". That's only correct if SetPluginName() ran before
  // PreInit() (see the comment in JOmniFactoryGeneratorT::GenerateFactories).
  // If that ordering regresses, GetPluginName() is empty during PreInit(),
  // m_prefix falls back to just the tag, and the parameter below is
  // registered as "test_tag:InputTags" instead.
  REQUIRE(app.GetJParameterManager()->Exists("test_plugin:test_tag:InputTags"));
}
