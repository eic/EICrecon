// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <ActsExamples/EventData/Track.hpp>
#include <ActsExamples/EventData/Trajectories.hpp>
#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/JException.h>
#include <exception>
#include <map>

#include "TrackProjector.h"
#include "TrackProjector_factory.h"
#include "datamodel_glue.h"
#include "services/geometry/acts/ACTSGeo_service.h"


namespace eicrecon {
    void TrackProjector_factory::Init() {
        // This prefix will be used for parameters
        std::string plugin_name = GetPluginName();                                                std::string param_prefix = plugin_name+ ":" + GetTag();

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(GetApplication(), param_prefix);

        auto acts_service = GetApplication()->GetService<ACTSGeo_service>();

        m_track_projector_algo.init(acts_service->actsGeoProvider(), logger());
    }

    void TrackProjector_factory::Process(const std::shared_ptr<const JEvent> &event) {
        // Now we check that user provided an input names
        auto trajectories = event->Get<ActsExamples::Trajectories>(GetInputTags()[0]);
        auto tracks = event->Get<ActsExamples::ConstTrackContainer>(GetInputTags()[1]);

        try {
            auto track_segments = m_track_projector_algo.execute(trajectories);
            SetCollection<edm4eic::TrackSegment>(GetOutputTags()[0], std::move(track_segments));
        }
        catch(std::exception &e) {
            throw JException(e.what());
        }
    }

} // eicrecon
