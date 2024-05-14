// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson

#pragma once

namespace eicrecon {

  struct TrackClusterMergeSplitterConfig {

    double minSigCut = -1.;  // min significance
    double avgEP     = 1.0;  // mean E/p
    double sigEP     = 1.0;  // rms of E/p
    double drAdd     = 0.4;  // window to add clusters

  };  // end TrackClusterMergeSplitterConfig

}  // end eicrecon namespace
