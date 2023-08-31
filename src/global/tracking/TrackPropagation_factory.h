// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Tyler Kutz

#pragma once

#include <extensions/jana/JChainMultifactoryT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <algorithms/tracking/TrackPropagation.h>
#include <spdlog/logger.h>

#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/TrackSegmentCollection.h>

#include <Acts/Surfaces/DiscSurface.hpp>
#include <Acts/Surfaces/CylinderSurface.hpp>

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
        std::shared_ptr<JDD4hep_service> m_geoSvc;

    private:
	
        eicrecon::TrackPropagation m_track_propagation_algo;

        std::vector<std::shared_ptr<Acts::Surface>> m_target_surface_list;
        std::vector<uint64_t> m_target_surface_ID;
        std::vector<uint32_t> m_target_detector_ID;

        void SetPropagationSurfaces();

};

} // eicrecon
