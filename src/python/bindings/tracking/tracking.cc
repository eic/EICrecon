// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Python extension module eicrecon._tracking: the tracking factory types
// (src/factories/tracking) for eicrecon.factories.tracking, and C++ helpers for config values that
// Python cannot write. Add one bind_factory line per factory type that a wiring uses.

#include <pybind11/pybind11.h>

#include "FactoryType.h"
#include "SubDivideFunctorBindings.h"
#include "factories/tracking/ActsToTracks_factory.h"
#include "factories/tracking/ActsTrackMerger_factory.h"
#include "factories/tracking/AmbiguitySolver_factory.h"
#include "factories/tracking/B0TrackSeeding_factory.h"
#include "factories/tracking/CKFTracking_factory.h"
#include "factories/tracking/IterativeVertexFinder_factory.h"
#include "factories/tracking/LGADHitClustering_factory.h"
#include "factories/tracking/MPGDHitReconstruction_factory.h"
#include "factories/tracking/SecondaryVertexFinder_factory.h"
#include "factories/tracking/TrackParamTruthInit_factory.h"
#include "factories/tracking/TrackProjector_factory.h"
#include "factories/tracking/TrackPropagation_factory.h"
#include "factories/tracking/TrackSeeding_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"
#include "factories/tracking/TrackerMeasurementFromHits_factory.h"
#include "factories/tracking/TracksToParticles_factory.h"
#include <edm4eic/MCRecoTrackParticleAssociation.h>
#include <edm4eic/MCRecoTrackParticleAssociationCollection.h>
#include <edm4eic/MCRecoTrackParticleLinkCollection.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/MCRecoTrackerHitLinkCollection.h>
#include <edm4eic/Measurement2D.h>
#include <edm4eic/RawTrackerHit.h>
#include <edm4eic/Track.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackerHit.h>
#include <edm4eic/TrackerHitCollection.h>
#include <edm4hep/SimTrackerHit.h>
#include <format> // IWYU pragma: keep
#include <podio/detail/Link.h>
#include <string>
#if IRT2_VERSION_MAJOR > 2 || (IRT2_VERSION_MAJOR == 2 && IRT2_VERSION_MINOR >= 2)
#include "ConfigConversion.h"
#include "RichIrtConfig.h"
#include <JANA/JApplication.h>
#endif

using namespace eicrecon;

namespace py = pybind11;
using eicrecon::python::bind_factory;

PYBIND11_MODULE(_tracking, m) {
  m.doc() = "EICrecon tracking factory types";
  py::module_::import("eicrecon._core");
  bind_factory<ActsToTracks_factory>(m, "ActsToTracks");
  bind_factory<ActsTrackMerger_factory>(m, "ActsTrackMerger");
  bind_factory<AmbiguitySolver_factory>(m, "AmbiguitySolver");
  bind_factory<B0TrackSeeding_factory>(m, "B0TrackSeeding");
  bind_factory<CKFTracking_factory>(m, "CKFTracking");
  bind_factory<IterativeVertexFinder_factory>(m, "IterativeVertexFinder");
  bind_factory<LGADHitClustering_factory>(m, "LGADHitClustering");
  bind_factory<MPGDHitReconstruction_factory>(m, "MPGDHitReconstruction");
  bind_factory<SecondaryVertexFinder_factory>(m, "SecondaryVertexFinder");
  bind_factory<TrackParamTruthInit_factory>(m, "TrackParamTruthInit");
  bind_factory<TrackProjector_factory>(m, "TrackProjector");
  bind_factory<TrackPropagation_factory>(m, "TrackPropagation");
  bind_factory<TrackSeeding_factory>(m, "TrackSeeding");
  bind_factory<TrackerHitReconstruction_factory>(m, "TrackerHitReconstruction");
  bind_factory<TrackerMeasurementFromHits_factory>(m, "TrackerMeasurementFromHits");
  bind_factory<TracksToParticles_factory>(m, "TracksToParticles");
}
