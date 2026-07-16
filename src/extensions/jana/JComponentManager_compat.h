// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#pragma once

#include <JANA/Services/JComponentManager.h>
#include <memory>

// JANA2 recently deprecated get_evt_srces() in favor of GetSources().
// Keep compatibility with older JANA2 while avoiding deprecation warnings
// on newer versions.
#if defined(JANA_VERSION_MAJOR) && defined(JANA_VERSION_MINOR) && defined(JANA_VERSION_PATCH) &&   \
    ((JANA_VERSION_MAJOR > 2) || (JANA_VERSION_MAJOR == 2 && JANA_VERSION_MINOR >= 5))
#define EICRECON_JANA_COMPONENT_MANAGER_HAS_GETSOURCES 1
#endif

namespace eicrecon::jana_compat {

inline std::vector<JEventSource*>& GetEventSources(JComponentManager* component_manager) {
#if defined(EICRECON_JANA_COMPONENT_MANAGER_HAS_GETSOURCES)
  return component_manager->GetSources();
#else
  return component_manager->get_evt_srces();
#endif
}

inline std::vector<JEventSource*>&
GetEventSources(const std::shared_ptr<JComponentManager>& component_manager) {
  return GetEventSources(component_manager.get());
}

} // namespace eicrecon::jana_compat
