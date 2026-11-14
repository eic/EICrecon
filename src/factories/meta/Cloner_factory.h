// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#pragma once

#include <JANA/JEvent.h>
#include <memory>
#include <string>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/meta/Cloner.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

template <typename T>
class Cloner_factory : public JOmniAlgorithmFactory<Cloner_factory<T>, Cloner<T>> {};

} // namespace eicrecon
