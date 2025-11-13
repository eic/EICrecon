// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/Track.h>
#include <edm4eic/TrackClusterMatchCollection.h>
#include <podio/ObjectID.h>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

// --------------------------------------------------------------------------
//! Algorithm input/output
// --------------------------------------------------------------------------
using ChargedCandidateMakerAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::TrackClusterMatchCollection>,
                          algorithms::Output<edm4eic::ReconstructedParticleCollection>>;

// ==========================================================================
//! Candidate Charged Particle Maker
// ==========================================================================
/*! An algorithm which takes a collection of track-cluster matches
 *  and converts them into charged-particle candidates, one for
 *  each track.
 */
class ChargedCandidateMaker : public ChargedCandidateMakerAlgorithm,
                              public WithPodConfig<NoConfig> {

public:
  // ------------------------------------------------------------------------
  //! Comparator struct for tracks
  // ------------------------------------------------------------------------
  /*! Organizes tracks by their ObjectIDs in decreasing collection
   *  ID first, and by decreasing index second.
   */
  struct CompareTrack {
    bool operator()(const edm4eic::Track& lhs, const edm4eic::Track& rhs) const {
      if (lhs.getObjectID().collectionID == rhs.getObjectID().collectionID) {
        return (lhs.getObjectID().index < rhs.getObjectID().index);
      } else {
        return (lhs.getObjectID().collectionID < rhs.getObjectID().collectionID);
      }
    }
  };

  ///! Alias for a map from a track to matched clusters
  using MapToVecClust = std::map<edm4eic::Track, std::vector<edm4eic::Cluster>, CompareTrack>;

  ///! Algorithm constructor
  ChargedCandidateMaker(std::string_view name)
      : ChargedCandidateMakerAlgorithm{name,
                                       {"inputTrackClusterMatches"},
                                       {"outputChargedCandidateParticles"},
                                       "Forms candidate charged particles."} {}

  // public method
  void process(const Input&, const Output&) const final;

}; // end ChargedCandidateMaker

} // namespace eicrecon
