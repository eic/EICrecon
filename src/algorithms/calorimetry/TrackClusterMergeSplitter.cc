// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson

#define EICRECON_TRACKCLUSTERMERGESPLITTER_CC

#include "TrackClusterMergeSplitter.h"

namespace eicrecon {

  // --------------------------------------------------------------------------
  //! Initialize algorithm
  // --------------------------------------------------------------------------
  void TrackClusterMergeSplitter::init() {

    /* TODO fill in */

  }  // end 'init()'

  // --------------------------------------------------------------------------
  //! Process inputs
  // --------------------------------------------------------------------------
  void TrackClusterMergeSplitter::process(
    const TrackClusterMergeSplitter::Input& input,
    const TrackClusterMergeSplitter::Output& output
  ) const {

    // grab inputs/outputs
    const auto [in_clusters, in_projections] = input;
    const auto [out_clusters] = output;

  }  // end 'process(Input&, Output&)'

}  // end eicrecon namespace
