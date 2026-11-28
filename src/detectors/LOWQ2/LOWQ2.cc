// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Python binding module eicrecon._wiring_LOWQ2: the factory types (and C++ helpers) used
// by the Python wiring of the LOWQ2 plugin, src/python/eicrecon/plugins/LOWQ2.py.
// Add one bind_factory line per factory type that the wiring uses.

#include <edm4eic/MCRecoTrackParticleAssociation.h>
#include <edm4eic/MCRecoTrackParticleLinkCollection.h>
#include <edm4eic/Track.h>
#include <edm4eic/TrackerHit.h>
#include <edm4hep/SimTrackerHit.h>
#include <podio/detail/Link.h>
#include <pybind11/pybind11.h>
#include <format> // IWYU pragma: keep
#include <string>

#include "FactoryType.h"
#include "factories/digi/PulseCombiner_factory.h"
#include "factories/digi/PulseGeneration_factory.h"
#include "factories/digi/PulseNoise_factory.h"
#include "factories/digi/SiliconChargeSharing_factory.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/fardetectors/FarDetectorLinearTracking_factory.h"
#include "factories/fardetectors/FarDetectorTrackerCluster_factory.h"
#include "factories/fardetectors/FarDetectorTransportationPostML_factory.h"
#include "factories/fardetectors/FarDetectorTransportationPreML_factory.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "factories/meta/ONNXInference_factory.h"
#include "factories/meta/SubDivideCollection_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"

using namespace eicrecon;

// Helpers for config values that Python cannot write

#include "SubDivideFunctorBindings.h"

inline void eicrecon_python_extra_LOWQ2(pybind11::module_& m) {
  eicrecon::python::bind_geometry_split<edm4eic::TrackerHit>(m, "GeometrySplit_TrackerHit");
}

namespace py = pybind11;
using eicrecon::python::bind_factory;

PYBIND11_MODULE(_wiring_LOWQ2, m) {
  m.doc() = "Factory types wired by the LOWQ2 plugin";
  py::module_::import("eicrecon._core");
  bind_factory<SiliconChargeSharing_factory>(m, "SiliconChargeSharing");
  bind_factory<PulseGeneration_factory<edm4hep::SimTrackerHit>>(m, "PulseGeneration_SimTrackerHit");
  bind_factory<PulseCombiner_factory>(m, "PulseCombiner");
  bind_factory<PulseNoise_factory>(m, "PulseNoise");
  bind_factory<SiliconTrackerDigi_factory>(m, "SiliconTrackerDigi");
  bind_factory<TrackerHitReconstruction_factory>(m, "TrackerHitReconstruction");
  bind_factory<SubDivideCollection_factory<edm4eic::TrackerHit>>(m,
                                                                 "SubDivideCollection_TrackerHit");
  bind_factory<FarDetectorTrackerCluster_factory>(m, "FarDetectorTrackerCluster");
  bind_factory<FarDetectorLinearTracking_factory>(m, "FarDetectorLinearTracking");
  bind_factory<CollectionCollector_factory<edm4eic::Track, true>>(m,
                                                                  "CollectionCollector_Track_true");
  bind_factory<CollectionCollector_factory<edm4eic::MCRecoTrackParticleLink, true>>(
      m, "CollectionCollector_MCRecoTrackParticleLink_true");
  bind_factory<CollectionCollector_factory<edm4eic::MCRecoTrackParticleAssociation, true>>(
      m, "CollectionCollector_MCRecoTrackParticleAssociation_true");
  bind_factory<FarDetectorTransportationPreML_factory>(m, "FarDetectorTransportationPreML");
  bind_factory<ONNXInference_factory>(m, "ONNXInference");
  bind_factory<FarDetectorTransportationPostML_factory>(m, "FarDetectorTransportationPostML");
  eicrecon_python_extra_LOWQ2(m);
}
