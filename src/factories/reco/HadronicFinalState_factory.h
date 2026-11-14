// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Tyler Kutz

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/HadronicFinalStateCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

template <typename AlgoT>
class HadronicFinalState_factory
    : public JOmniAlgorithmFactory<HadronicFinalState_factory<AlgoT>, AlgoT> {};

} // namespace eicrecon
