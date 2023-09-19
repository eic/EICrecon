
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Whitney Armstrong, Wouter Deconinck

#pragma once

#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <spdlog/logger.h>
#include <memory>

namespace eicrecon {

  class CalorimeterTruthClustering {

  protected:
    // Insert any member variables here
    std::shared_ptr<spdlog::logger> m_log;

  public:
    void init(std::shared_ptr<spdlog::logger> &logger);
    std::unique_ptr<edm4eic::ProtoClusterCollection> process(const edm4eic::CalorimeterHitCollection &hits, const edm4hep::SimCalorimeterHitCollection &mc);

  };

} // namespace eicrecon
