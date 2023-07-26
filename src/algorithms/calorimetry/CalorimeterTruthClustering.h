
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Whitney Armstrong, Wouter Deconinck

#pragma once

#include <random>

#include "services/geometry/dd4hep/JDD4hep_service.h"
#include <Evaluator/DD4hepUnits.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <spdlog/spdlog.h>



class CalorimeterTruthClustering {

protected:
    // Insert any member variables here
    std::shared_ptr<spdlog::logger> m_log;

public:
    CalorimeterTruthClustering() = default;
    void AlgorithmInit(std::shared_ptr<spdlog::logger> &logger);
    void AlgorithmChangeRun();
    std::unique_ptr<edm4eic::ProtoClusterCollection> AlgorithmProcess(const edm4eic::CalorimeterHitCollection &hits, const edm4hep::SimCalorimeterHitCollection &mc);
};
