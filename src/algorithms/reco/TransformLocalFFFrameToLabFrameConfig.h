// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Alex Jentsch
//

#pragma once

namespace eicrecon {

struct TransformLocalFFFrameToLabFrameConfig {

  /// Angle to rotate particles by. Defaults to
  /// usual 25 mrad crossing angle.
  double m_crossing_angle = -0.025;
};

} // namespace eicrecon
