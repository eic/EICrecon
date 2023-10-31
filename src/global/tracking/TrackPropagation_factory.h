// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Tyler Kutz

#pragma once

#include <Acts/Surfaces/Surface.hpp>
#include <JANA/JEvent.h>
#include <algorithms/tracking/TrackPropagation.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <extensions/jana/JChainMultifactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <services/geometry/dd4hep/DD4hep_service.h>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

    class TrackPropagation_factory : public JChainMultifactoryT<NoConfig>,
                                     public SpdlogMixin {

    public:

        explicit TrackPropagation_factory(std::string tag,
                        const std::vector<std::string>& input_tags,
                        const std::vector<std::string>& output_tags) :
            JChainMultifactoryT<NoConfig>(std::move(tag), input_tags, output_tags) {
                DeclarePodioOutput<edm4eic::TrackSegment>(GetOutputTags()[0]);
        }

        /** One time initialization **/
        void Init() override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

        // Pointer to the geometry service
        std::shared_ptr<DD4hep_service> m_geoSvc;

    private:

        eicrecon::TrackPropagation m_track_propagation_algo;

        std::vector<std::shared_ptr<Acts::Surface>> m_target_surface_list;
        std::vector<uint64_t> m_target_surface_ID;
        std::vector<uint32_t> m_target_detector_ID;

        void SetPropagationSurfaces();

};

} // eicrecon
