// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright 2024, Dmitry Kalinkin

#pragma once

#include <edm4eic/EDM4eicVersion.h>
#include <cassert>
#include <memory>

#include "algorithms/tracking/ActsToTracks.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class ActsToTracks_factory
    : public JOmniAlgorithmFactory<ActsToTracks_factory, eicrecon::ActsToTracks, NoConfig> {};

} // namespace eicrecon
