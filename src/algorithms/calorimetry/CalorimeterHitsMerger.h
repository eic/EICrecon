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
#include <memory>
#include <tuple>
#include <unordered_map>

#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <Evaluator/DD4hepUnits.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/vector_utils.h>
#include <spdlog/spdlog.h>


class CalorimeterHitsMerger  {
public:

    std::string m_readout;
    std::vector<std::string> u_fields;
    std::vector<int> u_refs;

    std::shared_ptr<JDD4hep_service> m_geoSvc;
    uint64_t id_mask{0}, ref_mask{0};

    std::shared_ptr<spdlog::logger> m_log;

public:
    CalorimeterHitsMerger() = default;
    virtual ~CalorimeterHitsMerger() {}

    virtual void initialize();
    std::unique_ptr<edm4eic::CalorimeterHitCollection> execute(const edm4eic::CalorimeterHitCollection &input);

}; // class CalorimeterHitsMerger
