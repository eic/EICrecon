// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#include "ActsTrackMerger.h"

#include <Acts/EventData/MeasurementHelpers.hpp>
#include <Acts/EventData/TrackContainer.hpp>
#include <Acts/EventData/TrackProxy.hpp>
#include <Acts/EventData/TrackStatePropMask.hpp>
#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <ActsPlugins/EDM4hep/PodioTrackContainer.hpp>
#include <ActsPlugins/EDM4hep/PodioTrackStateContainer.hpp>
#include <ActsPodioEdm/BoundParametersCollection.h>
#include <ActsPodioEdm/JacobianCollection.h>
#include <ActsPodioEdm/TrackCollection.h>
#include <ActsPodioEdm/TrackStateCollection.h>
#include <Eigen/LU> // IWYU pragma: keep
#include <any>
#include <gsl/pointers>
#include <memory>
#include <utility>
#include <vector>

#include "algorithms/tracking/PodioGeometryIdConversionHelper.h"

namespace eicrecon {

using ConstPodioTrackContainer =
    Acts::TrackContainer<ActsPlugins::ConstPodioTrackContainer<>,
                         ActsPlugins::ConstPodioTrackStateContainer<>, std::shared_ptr>;
using MutablePodioTrackContainer =
    Acts::TrackContainer<ActsPlugins::MutablePodioTrackContainer<Acts::RefHolder>,
                         ActsPlugins::MutablePodioTrackStateContainer<Acts::RefHolder>,
                         std::shared_ptr>;

void ActsTrackMerger::process(const Input& input, const Output& output) const {
  const auto [track_states1, track_params1, track_jac1, tracks1, track_states2, track_params2,
              track_jac2, tracks2]                                     = input;
  auto [out_track_states, out_track_params, out_track_jac, out_tracks] = output;

  // Create conversion helper for Podio backend
  PodioGeometryIdConversionHelper helper(m_geoSvc->getActsGeometryContext(),
                                         m_geoSvc->trackingGeometry());

  // Collect all input track containers by reconstructing ConstTrackContainer wrappers
  std::vector<ConstPodioTrackContainer> input_containers;

  // Process first input set (const inputs) - use default ConstRefHolder template parameter
  // ConstRefHolder<const T> wraps a const collection reference
  // Input collections are gsl::not_null pointers, so we dereference them
  // The holders need to be explicitly constructed
  auto trackStateContainer1 = std::make_shared<ActsPlugins::ConstPodioTrackStateContainer<>>(
      helper, Acts::ConstRefHolder<const ActsPodioEdm::TrackStateCollection>{*track_states1},
      Acts::ConstRefHolder<const ActsPodioEdm::BoundParametersCollection>{*track_params1},
      Acts::ConstRefHolder<const ActsPodioEdm::JacobianCollection>{*track_jac1});
  auto trackContainer1 = std::make_shared<ActsPlugins::ConstPodioTrackContainer<>>(
      helper, Acts::ConstRefHolder<const ActsPodioEdm::TrackCollection>{*tracks1});
  input_containers.emplace_back(trackContainer1, trackStateContainer1);

  // Process second input set (const inputs) - use default ConstRefHolder template parameter
  auto trackStateContainer2 = std::make_shared<ActsPlugins::ConstPodioTrackStateContainer<>>(
      helper, Acts::ConstRefHolder<const ActsPodioEdm::TrackStateCollection>{*track_states2},
      Acts::ConstRefHolder<const ActsPodioEdm::BoundParametersCollection>{*track_params2},
      Acts::ConstRefHolder<const ActsPodioEdm::JacobianCollection>{*track_jac2});
  auto trackContainer2 = std::make_shared<ActsPlugins::ConstPodioTrackContainer<>>(
      helper, Acts::ConstRefHolder<const ActsPodioEdm::TrackCollection>{*tracks2});
  input_containers.emplace_back(trackContainer2, trackStateContainer2);

  // Create new mutable containers for merging (mutable outputs)
  // RefHolder<T> wraps a non-const collection reference
  auto mergedTrackStateContainer =
      std::make_shared<ActsPlugins::MutablePodioTrackStateContainer<Acts::RefHolder>>(
          helper, Acts::RefHolder<ActsPodioEdm::TrackStateCollection>{*out_track_states},
          Acts::RefHolder<ActsPodioEdm::BoundParametersCollection>{*out_track_params},
          Acts::RefHolder<ActsPodioEdm::JacobianCollection>{*out_track_jac});
  auto mergedTrackContainer =
      std::make_shared<ActsPlugins::MutablePodioTrackContainer<Acts::RefHolder>>(
          helper, Acts::RefHolder<ActsPodioEdm::TrackCollection>{*out_tracks});
  MutablePodioTrackContainer mergedTracks(mergedTrackContainer, mergedTrackStateContainer);

  // Copy all tracks from all input containers
  for (const auto& inputContainer : input_containers) {
    // Copy each track (copyFrom handles dynamic columns internally)
    for (const auto& srcTrack : inputContainer) {
      auto destTrack = mergedTracks.makeTrack();
      destTrack.copyFrom(srcTrack);
    }
  }
}

} // namespace eicrecon
