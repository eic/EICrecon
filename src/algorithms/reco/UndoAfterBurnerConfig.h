// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Alex Jentsch, Jihee Kim, Brian Page
//

#pragma once

namespace eicrecon {

struct UndoAfterBurnerConfig {

  bool m_pid_assume_pion_mass = false;
  double m_crossing_angle     = -0.025;
  double m_pid_purity         = 0.51;
  bool m_correct_beam_FX      = true;
  bool m_pid_use_MC_truth     = true;
  int m_max_gen_status        = -1; // Upper limit on generator status to process (-1 = no limit)
};

} // namespace eicrecon
