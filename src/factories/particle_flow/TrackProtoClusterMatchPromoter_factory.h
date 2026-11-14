// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#pragma once

#include <edm4eic/EDM4eicVersion.h>

#include "algorithms/particle_flow/TrackProtoClusterMatchPromoter.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class TrackProtoClusterMatchPromoter_factory
    : public JOmniAlgorithmFactory<TrackProtoClusterMatchPromoter_factory,
                                   eicrecon::TrackProtoClusterMatchPromoter> {
}; // end TrackProtoClusterMatchPromoter_factory

} // namespace eicrecon
