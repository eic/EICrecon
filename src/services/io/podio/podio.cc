// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplicationFwd.h>
#include <JANA/JEventSourceGeneratorT.h>
#include <JANA/Services/JParameterManager.h>
#include <string>

#include "JEventProcessorManagedPODIO.h"
#include "JEventProcessorPODIO.h"
#include "JEventSourceManagedPODIO.h"
#include "JEventSourcePODIO.h"

namespace {

template <typename SourceT>
class LeveledEventSourceGeneratorT : public JEventSourceGeneratorT<SourceT> {
public:
  explicit LeveledEventSourceGeneratorT(JEventLevel level) { this->SetLevel(level); }
};

} // namespace

// Make this a JANA plugin
extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  const bool split_timeframes =
      app->RegisterParameter<bool>("split_timeframes", false, "Enable timeframe splitting");
  const auto source_level = split_timeframes ? JEventLevel::Timeslice : JEventLevel::PhysicsEvent;

  // Check if managed mode is requested
  if (app->GetJParameterManager()->Exists("podio:managed_socket_path")) {
    auto* source = new JEventSourceManagedPODIO("", app);
    source->SetLevel(source_level);
    app->Add(source);
    app->Add(new JEventProcessorManagedPODIO());
  } else {
    app->Add(new LeveledEventSourceGeneratorT<JEventSourcePODIO>(source_level));
    app->Add(new JEventProcessorPODIO());
  }
}
}
