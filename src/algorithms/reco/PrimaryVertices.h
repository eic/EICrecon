// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Daniel Brandenburg

#pragma once

#include <edm4eic/VertexCollection.h>
#include <spdlog/logger.h>
#include <memory>

#include "algorithms/reco/PrimaryVerticesConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"


namespace eicrecon {

    class PrimaryVertices : public WithPodConfig<PrimaryVerticesConfig>{

    public:

        void init(std::shared_ptr<spdlog::logger>& logger);
        std::unique_ptr<edm4eic::VertexCollection> execute(
                const edm4eic::VertexCollection *rcvtx
        );

    private:
        std::shared_ptr<spdlog::logger> m_log;

    };
} // namespace eicrecon
