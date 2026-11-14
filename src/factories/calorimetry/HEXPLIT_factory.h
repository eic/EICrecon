// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Sebouh Paul

#pragma once

#include "algorithms/calorimetry/HEXPLIT.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class HEXPLIT_factory
    : public JOmniAlgorithmFactory<HEXPLIT_factory, eicrecon::HEXPLIT, HEXPLITConfig> {
private:
  ParameterRef<double> m_MIP{this, "MIP", config().MIP};
  ParameterRef<double> m_Emin_in_MIPs{this, "Emin_in_MIPs", config().Emin_in_MIPs};
  ParameterRef<double> m_delta_in_MIPs{this, "delta_in_MIPs", config().delta_in_MIPs};
};

} // namespace eicrecon
