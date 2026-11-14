// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Dmitry Kalinkin

#pragma once

#include <edm4eic/EDM4eicVersion.h>
#include "algorithms/onnx/CalorimeterParticleIDPostML.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class CalorimeterParticleIDPostML_factory
    : public JOmniAlgorithmFactory<CalorimeterParticleIDPostML_factory,
                                   eicrecon::CalorimeterParticleIDPostML, NoConfig> {};

} // namespace eicrecon
