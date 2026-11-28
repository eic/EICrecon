// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Python binding module eicrecon._wiring_tracking: the factory types (and C++ helpers) used
// by the Python wiring of the tracking plugin, src/python/eicrecon/plugins/tracking.py.
// Add one bind_factory line per factory type that the wiring uses.

#include <edm4eic/MCRecoTrackParticleAssociationCollection.h>
#include <edm4eic/MCRecoTrackParticleLinkCollection.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/MCRecoTrackerHitLinkCollection.h>
#include <edm4eic/Measurement2D.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <podio/detail/Link.h>
#include <pybind11/pybind11.h>
#include <string>

#include "FactoryType.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "factories/tracking/ActsToTracks_factory.h"
#include "factories/tracking/ActsTrackMerger_factory.h"
#include "factories/tracking/AmbiguitySolver_factory.h"
#include "factories/tracking/B0TrackSeeding_factory.h"
#include "factories/tracking/CKFTracking_factory.h"
#include "factories/tracking/IterativeVertexFinder_factory.h"
#include "factories/tracking/SecondaryVertexFinder_factory.h"
#include "factories/tracking/TrackParamTruthInit_factory.h"
#include "factories/tracking/TrackProjector_factory.h"
#include "factories/tracking/TrackPropagation_factory.h"
#include "factories/tracking/TrackSeeding_factory.h"
#include "factories/tracking/TrackerMeasurementFromHits_factory.h"
#include "factories/tracking/TracksToParticles_factory.h"

using namespace eicrecon;

namespace py = pybind11;
using eicrecon::python::bind_factory;

PYBIND11_MODULE(_wiring_tracking, m) {
  m.doc() = "Factory types wired by the tracking plugin";
  py::module_::import("eicrecon._core");
  bind_factory<TrackParamTruthInit_factory>(m, "TrackParamTruthInit");
  bind_factory<CollectionCollector_factory<edm4eic::TrackerHit, true>>(
      m, "CollectionCollector_TrackerHit_true");
  bind_factory<CollectionCollector_factory<edm4eic::MCRecoTrackerHitAssociation, true>>(
      m, "CollectionCollector_MCRecoTrackerHitAssociation_true");
  bind_factory<CollectionCollector_factory<edm4eic::MCRecoTrackerHitLink, true>>(
      m, "CollectionCollector_MCRecoTrackerHitLink_true");
  bind_factory<TrackerMeasurementFromHits_factory>(m, "TrackerMeasurementFromHits");
  bind_factory<CollectionCollector_factory<edm4eic::Measurement2D, true>>(
      m, "CollectionCollector_Measurement2D_true");
  bind_factory<CKFTracking_factory>(m, "CKFTracking");
  bind_factory<ActsToTracks_factory>(m, "ActsToTracks");
  bind_factory<AmbiguitySolver_factory>(m, "AmbiguitySolver");
  bind_factory<TrackSeeding_factory>(m, "TrackSeeding");
  bind_factory<TrackProjector_factory>(m, "TrackProjector");
  bind_factory<IterativeVertexFinder_factory>(m, "IterativeVertexFinder");
  bind_factory<TrackPropagation_factory>(m, "TrackPropagation");
  bind_factory<B0TrackSeeding_factory>(m, "B0TrackSeeding");
  bind_factory<ActsTrackMerger_factory>(m, "ActsTrackMerger");
  bind_factory<CollectionCollector_factory<edm4eic::Track, true>>(m,
                                                                  "CollectionCollector_Track_true");
  bind_factory<SecondaryVertexFinder_factory>(m, "SecondaryVertexFinder");
  bind_factory<CollectionCollector_factory<edm4eic::MCRecoTrackParticleAssociation, true>>(
      m, "CollectionCollector_MCRecoTrackParticleAssociation_true");
  bind_factory<CollectionCollector_factory<edm4eic::MCRecoTrackParticleLink, true>>(
      m, "CollectionCollector_MCRecoTrackParticleLink_true");
  bind_factory<TracksToParticles_factory>(m, "TracksToParticles");
}
