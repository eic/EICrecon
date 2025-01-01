#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/ReconstructedParticle.h>
#include <edm4hep/utils/vector_utils.h>

#include "algorithms/reco/TrackClusterMatch.h"
#include "algorithms/reco/TrackClusterMatchConfig.h"

namespace eicrecon {
    /*
    * @brief Initialize the TrackClusterMatch Algorithm
    */
   void TrackClusterMatch::init() {}

    /*
     * @brief Produce a list of scattered electron candidates
     *
     * @param tracks - input collection of all tracks
     * @param clusters  - input collection of all clusters
     * @return std::unique_ptr<edm4eic::ReconstructedParticleCollection>
     */
    void TrackClusterMatch::process(
        const TrackClusterMatch::Input& input,
        const TrackClusterMatch::Output& output
    ) const {
        auto [clusters, tracks] = input;
        auto [matched] = output;

        // Loop over each cluster
        for (const auto& cluster: *clusters) {
            // Get the position of the cluster
            auto cluster_eta = edm4hep::utils::eta(cluster.getPosition());
            auto cluster_phi = edm4hep::utils::angleAzimuthal(cluster.getPosition());
            float min_distance = TrackClusterMatchConfig().maxMatchDistance;

            // Loop over each track
            std::optional<edm4eic::TrackPoint> match;
            for (const auto& track: *tracks) {
                for (const auto& projection: track.getPoints()) {
                    // Get the position of the projection
                    auto projection_eta = edm4hep::utils::eta(projection.position);
                    auto projection_phi = edm4hep::utils::angleAzimuthal(projection.position);
                    auto this_distance = std::hypot(cluster_eta - projection_eta, cluster_phi - projection_phi);
                    // Update the match if there is a closer distance
                    if (this_distance < min_distance) {
                        min_distance = this_distance;
                        match = projection;
                    }
                }
            }
            // Check if we found a match, and if so make a reconstructed particle with the two
            if (match.has_value()) {
                std::cout << "Yay!  Match found!" << std::endl;

            }
        }
    }
}