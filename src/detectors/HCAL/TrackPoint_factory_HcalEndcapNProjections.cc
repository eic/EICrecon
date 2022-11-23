#include <Acts/Surfaces/DiscSurface.hpp>
#include <Acts/Surfaces/RadialBounds.hpp>



#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <Math/GenVector/PxPyPzM4D.h>

#include <spdlog/spdlog.h>

#include <algorithms/tracking/ParticlesFromTrackFitResult.h>
#include <services/rootfile/RootFile_service.h>
#include <services/geometry/acts/ACTSGeo_service.h>
#include "TrackPoint_factory_HcalEndcapNProjections.h"
#include "extensions/string/StringHelpers.h"


//------------------------------------------
// Init
void TrackPoint_factory_HcalEndcapNProjections::Init() {
    std::string plugin_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");
    std::string param_prefix = plugin_name + ":" + GetTag();

    // Get JANA application
    auto app = GetApplication();


    // Get log level from user parameter or default
    InitLogger(plugin_name);

    auto acts_service = GetApplication()->GetService<ACTSGeo_service>();

    m_propagation_algo.init(acts_service->actsGeoProvider(), logger());

    // Create HCal surface that will be used for propagation
    auto transform = Acts::Transform3::Identity();

    // make a reference disk to mimic electron-endcap HCal
    double hcalEndcapNZ = -3322.;
    double hcalEndcapNMinR = 83.01;
    double hcalEndcapNMaxR = 950;

    // Check if users ha
    app->SetDefaultParameter(param_prefix + ":plainZ", hcalEndcapNZ, "Projection plane Z");

    auto hcalEndcapNBounds = std::make_shared<Acts::RadialBounds>(hcalEndcapNMinR, hcalEndcapNMaxR);
    auto hcalEndcapNTrf = transform * Acts::Translation3(Acts::Vector3(0, 0, hcalEndcapNZ));
    m_hcal_surface = Acts::Surface::makeShared<Acts::DiscSurface>(hcalEndcapNTrf, hcalEndcapNBounds);
}

//------------------------------------------
// Process
void TrackPoint_factory_HcalEndcapNProjections::Process(const std::shared_ptr<const JEvent> &event) {
    m_log->trace("TrackPoint_factory_HcalEndcapNProjections event");

// Get trajectories from tracking
    auto trajectories = event->Get<eicrecon::TrackingResultTrajectory>("CentralCKFTrajectories");
    std::vector<edm4eic::TrackPoint *> result_poins;

// Iterate over trajectories
    m_log->debug("Propagating through {} trajectories", trajectories.

            size()

    );
    for (
            size_t traj_index = 0;
            traj_index < trajectories.

                    size();

            traj_index++) {
        auto &trajectory = trajectories[traj_index];
        m_log->trace(" -- trajectory {} --", traj_index);

        edm4eic::TrackPoint *projection_point;
        try {
// >>> try to propagate to surface <<<
            projection_point = m_propagation_algo.propagate(trajectory, m_hcal_surface);
        }
        catch (
                std::exception &e
        ) {
            m_log->warn("Exception in underlying algorithm: {}. Trajectory is skipped", e.

                    what()

            );
        }

        if (!projection_point) {
            m_log->trace("   could not propagate!", traj_index);
            continue;
        }

// Now go through reconstructed tracks points
        auto pos = projection_point->position;
        auto length = projection_point->pathlength;
        m_log->trace("   {:>10} {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f}", traj_index, pos.x, pos.y, pos.z, length);

        result_poins.
                push_back(projection_point);
    }

// Put data as a factory running result
    Set(result_poins);
}




