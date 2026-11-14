// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright 2022 - 2025 Dmitry Romanov, Dmitry Kalinkin

#pragma once

#include <ActsExamples/EventData/Track.hpp>
#include <JANA/JEvent.h>
#include <cassert>
#include <edm4eic/TrackSegmentCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/tracking/TrackProjector.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class TrackProjector_factory
    : public JOmniAlgorithmFactory<TrackProjector_factory, eicrecon::TrackProjector, NoConfig> {};

} // namespace eicrecon
