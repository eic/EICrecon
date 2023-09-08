
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Whitney Armstrong, Wouter Deconinck

#pragma once

#include <random>

#include <DD4hep/Detector.h>
#include <Evaluator/DD4hepUnits.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <spdlog/spdlog.h>

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
