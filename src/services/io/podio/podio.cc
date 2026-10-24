// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/JEventSourceGeneratorT.h>
#include <JANA/Services/JParameterManager.h>
#include <JANA/Utils/JEventLevel.h>
#include <string>

#include "JEventProcessorManagedPODIO.h"
#include "JEventProcessorPODIO.h"
#include "JEventSourceManagedPODIO.h"
#include "JEventSourcePODIO.h"

// JEventSourceGenerator::SetLevel() is public since JANA2 v2026.03.01
// (JeffersonLab/JANA2@7539bd45). TODO: Drop the fallback once that version is required.
#if (JANA_VERSION_MAJOR > 2026) || (JANA_VERSION_MAJOR == 2026 && JANA_VERSION_MINOR > 3) ||       \
    (JANA_VERSION_MAJOR == 2026 && JANA_VERSION_MINOR == 3 && JANA_VERSION_PATCH >= 1)
#define EICRECON_JANA_HAS_PUBLIC_JESG_SETLEVEL 1
#else
namespace {

template <typename SourceT>
class LeveledEventSourceGeneratorT : public JEventSourceGeneratorT<SourceT> {
public:
  explicit LeveledEventSourceGeneratorT(JEventLevel level) { this->SetLevel(level); }
};

} // namespace
#endif

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
#ifdef EICRECON_JANA_HAS_PUBLIC_JESG_SETLEVEL
    auto* source_generator = new JEventSourceGeneratorT<JEventSourcePODIO>();
    source_generator->SetLevel(source_level);
    app->Add(source_generator);
#else
    app->Add(new LeveledEventSourceGeneratorT<JEventSourcePODIO>(source_level));
#endif
    app->Add(new JEventProcessorPODIO());
  }
}
}
