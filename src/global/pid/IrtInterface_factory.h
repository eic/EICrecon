// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, 2025, 2026, Alexander Kiselev

#pragma once

#include <JANA/JEvent.h>
#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/pid/IrtInterface.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

class CherenkovDetector;

#include <edm4eic/RawTrackerHitCollection.h>

namespace eicrecon {

class IrtInterface_factory
    : public JOmniAlgorithmFactory<IrtInterface_factory, eicrecon::IrtInterface, IrtConfig> {};

} // namespace eicrecon
