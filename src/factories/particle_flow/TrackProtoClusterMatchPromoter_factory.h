// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2025, Wouter Deconinck

#pragma once

#include "extensions/jana/JOmniFactory.h"

#ifndef EICRECON_FACTORY_PRECOMPILE

namespace eicrecon {
class TrackProtoClusterMatchPromoter_factory;
}

extern template class JOmniFactory<eicrecon::TrackProtoClusterMatchPromoter_factory>;

#else

#include "extensions/spdlog/SpdlogExtensions.h"
#include <edm4eic/Cluster.h>
#include <edm4eic/ProtoCluster.h>
#include <edm4eic/TrackPoint.h>

namespace eicrecon {

class TrackProtoClusterMatchPromoter_factory
    : public JOmniFactory<TrackProtoClusterMatchPromoter_factory> {

public:
  using Input  = std::tuple<edm4eic::TrackPoint, edm4eic::Cluster>;
  using Output = edm4eic::ProtoCluster;

  void Process(const Input& input, Output& output) const;
};

}  // namespace eicrecon

#endif
