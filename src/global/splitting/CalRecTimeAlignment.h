// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024 Whitney Armstrong, Wouter Deconinck, Dmitry Romanov

#pragma once

#include <edm4eic/CalorimeterHitCollection.h>
#include <string_view>

#include "RecHitTimeAlignment.h"

namespace eicrecon {

class CalRecTimeAlignment
    : public RecHitTimeAlignment<edm4eic::CalorimeterHitCollection,
                                 edm4eic::MutableCalorimeterHit> {
public:
  CalRecTimeAlignment(std::string_view name)
      : RecHitTimeAlignment{name,
                            "inputCalorimeterHits",
                            "outputCalorimeterHits",
                            "Align reconstructed calorimeter hit times."} {}
};

} // namespace eicrecon
