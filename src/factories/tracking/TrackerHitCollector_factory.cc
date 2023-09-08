// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#include <JANA/JEvent.h>
#include <edm4eic/TrackerHitCollection.h>

#include "TrackerHitCollector_factory.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"

namespace eicrecon {
    void TrackerHitCollector_factory::Init() {
        auto app = GetApplication();

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(app, GetPrefix(), "info");

        m_algo.init(logger());
    }

    void TrackerHitCollector_factory::Process(const std::shared_ptr<const JEvent> &event) {
        std::vector<const edm4eic::TrackerHitCollection*> hit_collections;
        for (const auto& input_tag : GetInputTags()) {
            try {
                hit_collections.emplace_back(event->GetCollection<edm4eic::TrackerHit>(input_tag));
            }
            catch(std::exception &e) {
                // ignore missing collections, but print them in debug mode
                m_log->debug(e.what());
            }
        }

        try {
            auto hits = m_algo.process(hit_collections);
            SetCollection<edm4eic::TrackerHit>(GetOutputTags()[0], std::move(hits));
        }
        catch(std::exception &e) {
            throw JException(e.what());
        }
    }

} // eicrecon
