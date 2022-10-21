// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include "TrackProjector_factory.h"
#include "extensions/string/StringHelpers.h"
#include "algorithms/tracking/JugTrack/Trajectories.hpp"
#include "services/geometry/acts/ACTSGeo_service.h"


namespace eicrecon {
    void TrackProjector_factory::Init() {
        // This prefix will be used for parameters
        std::string plugin_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");
        std::string param_prefix = plugin_name+ ":" + GetTag();

        // Set input data tags properly
        InitDataTags(param_prefix);

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(param_prefix, "info");

        auto acts_service = GetApplication()->GetService<ACTSGeo_service>();

        m_track_projector_algo.init(acts_service->actsGeoProvider(), logger());
    }

    void TrackProjector_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
        JFactoryT::ChangeRun(event);
    }

    void TrackProjector_factory::Process(const std::shared_ptr<const JEvent> &event) {
        // Now we check that user provided an input names
        std::string input_tag = GetInputTags()[0];

        // Collect all hits
        auto trajectories = event->Get<Jug::Trajectories>(input_tag);
        auto result = m_track_projector_algo.execute(trajectories);
        Set(result);
    }

} // eicrecon