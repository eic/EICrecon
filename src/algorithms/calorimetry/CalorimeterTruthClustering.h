
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Whitney Armstrong, Wouter Deconinck

#pragma once

#include <memory>

namespace edm4eic { class CalorimeterHitCollection; }
namespace edm4eic { class ProtoClusterCollection; }
namespace edm4hep { class SimCalorimeterHitCollection; }
namespace spdlog { class logger; }

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
