// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#include <ActsExamples/EventData/Track.hpp>
#include <JANA/JEvent.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/tracking/ActsTrackMerger.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

/// Factory that merges Acts track containers from multiple sources into a single output.
///
/// Typical use is to combine tracks reconstructed in different subsystems
/// (e.g. central tracker and B0 tracker) into one unified collection for
/// downstream reconstruction or analysis. The current implementation simply
/// concatenates the input ConstTrackContainer collections in the order they
/// are provided.
class ActsTrackMerger_factory
    : public JOmniAlgorithmFactory<ActsTrackMerger_factory, eicrecon::ActsTrackMerger, NoConfig> {};

} // namespace eicrecon
