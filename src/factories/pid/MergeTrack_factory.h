// Copyright (C) 2022 - 2024 Christopher Dilks, Wouter Deconinck
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// algorithms
#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/pid/MergeTracks.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class MergeTrack_factory
    : public JOmniAlgorithmFactory<MergeTrack_factory, eicrecon::MergeTracks, NoConfig> {};
} // namespace eicrecon
