// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Jihee Kim, Sylvester Joosten, Whitney Armstrong, Wouter Deconinck, David Lawrence

/*
 *  An algorithm to group readout hits from a calorimeter
 *  Energy is summed
 *
 *  Author: Chao Peng (ANL), 03/31/2021
 */

#pragma once

#include <algorithm>
#include <bitset>
#include <tuple>
#include <unordered_map>

#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <Evaluator/DD4hepUnits.h>
#include <edm4eic/RawCalorimeterHit.h>
#include <edm4eic/CalorimeterHit.h>
#include <edm4eic/vector_utils.h>
#include <spdlog/spdlog.h>

using namespace dd4hep;

class CalorimeterHitsMerger  {
public:

    std::string m_readout;
    std::vector<std::string> u_fields;
    std::vector<int> u_refs;

    std::shared_ptr<JDD4hep_service> m_geoSvc;
    uint64_t id_mask{0}, ref_mask{0};

    std::vector<const edm4eic::CalorimeterHit *> m_inputs;
    std::vector<edm4eic::CalorimeterHit *> m_outputs;

    std::shared_ptr<spdlog::logger> m_log;

public:
    CalorimeterHitsMerger() = default;
    virtual ~CalorimeterHitsMerger() {}

    virtual void initialize();
    virtual void execute();

}; // class CalorimeterHitsMerger

