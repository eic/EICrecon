// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024 Whitney Armstrong, Wouter Deconinck, Dmitry Romanov

#pragma once

#include <edm4eic/TrackerHitCollection.h>
#include <string_view>

#include "RecHitTimeAlignment.h"

namespace eicrecon {

class TrkTimeAlignment
    : public RecHitTimeAlignment<edm4eic::TrackerHitCollection, edm4eic::MutableTrackerHit> {
public:
  TrkTimeAlignment(std::string_view name)
      : RecHitTimeAlignment{name, "inputTrackerHits", "outputTrackerHits",
                            "Align reconstructed tracker hit times."} {}
};

} // namespace eicrecon
