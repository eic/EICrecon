// Created by Tyler Kutz
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <services/io/podio/JFactoryPodioT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include "extensions/spdlog/SpdlogMixin.h"
#include <algorithms/tracking/TrackPropagation.h>
#include <spdlog/logger.h>

#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/TrackSegmentCollection.h>

#include <Acts/Surfaces/DiscSurface.hpp>
#include <Acts/Surfaces/CylinderSurface.hpp>

namespace eicrecon {

	class TrackPropagation_factory : public JFactoryPodioT<edm4eic::TrackSegment>,
					 public SpdlogMixin {

	    public:
		
		std::string m_input_tag;
			    
		explicit TrackPropagation_factory() {
			SetTag("PropagatedTrackPoints");
		}

		/** One time initialization **/
		void Init() override;

		/** On run change preparations **/
		void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

		/** Event by event processing **/
		void Process(const std::shared_ptr<const JEvent> &event) override;
	    
		// Pointer to the geometry service
		std::shared_ptr<JDD4hep_service> m_geoSvc;
		dd4hep::IDDescriptor m_idSpec;

	    private:
		eicrecon::TrackPropagation m_track_propagation_algo;

		std::vector<std::shared_ptr<Acts::Surface>> m_target_surface_list;
		std::vector<int32_t> m_target_surface_ID;

		void SetPropagationSurfaces();

	};

} // eicrecon

