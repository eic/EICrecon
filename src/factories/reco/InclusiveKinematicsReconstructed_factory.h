// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

template <typename AlgoT>
class InclusiveKinematicsReconstructed_factory
    : public JOmniAlgorithmFactory<InclusiveKinematicsReconstructed_factory<AlgoT>, AlgoT> {};

} // namespace eicrecon
