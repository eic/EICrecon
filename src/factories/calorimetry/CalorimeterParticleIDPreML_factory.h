// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Dmitry Kalinkin

#pragma once

#include "algorithms/onnx/CalorimeterParticleIDPreML.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class CalorimeterParticleIDPreML_factory
    : public JOmniAlgorithmFactory<CalorimeterParticleIDPreML_factory,
                                   eicrecon::CalorimeterParticleIDPreML, NoConfig> {};

} // namespace eicrecon
