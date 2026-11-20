// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#pragma once

namespace eicrecon {

struct ClonerConfig {
  /// Clone relations (e.g. MCParticle parents/daughters) along with the elements.
  /// Disable to obtain self-contained objects that do not reference other collections.
  bool cloneRelations{true};
};

} // namespace eicrecon
