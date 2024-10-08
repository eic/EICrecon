// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson

#include <edm4eic/CalorimeterHit.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <stdint.h>
#include <cmath>
#include <cstddef>
#include <gsl/pointers>

// algorithm definition
#include "TrackClusterSubtraction.h"
#include "algorithms/calorimetry/TrackClusterSubtractionConfig.h"



namespace eicrecon {

  // --------------------------------------------------------------------------
  //! Initialize algorithm
  // --------------------------------------------------------------------------
  void TrackClusterSubtraction::init(const dd4hep::Detector* detector) {

    // grab detector id
    m_idCalo = detector -> constant<int>(m_cfg.idCalo);
    debug("Collecting projections to detector with system id {}", m_idCalo);

  }  // end 'init(dd4hep::Detector*)'



  // --------------------------------------------------------------------------
  //! Process inputs
  // --------------------------------------------------------------------------
  /*! TODO fill in
   */
  void TrackClusterSubtraction::process(
    const TrackClusterSubtraction::Input& input,
    const TrackClusterSubtraction::Output& output
  ) const {

    /* TODO fill in */

  }  // end 'get_projections(edm4eic::CalorimeterHit&, edm4eic::TrackSegmentCollection&, VecTrkPoint&)'

}  // end eicrecon namespace
