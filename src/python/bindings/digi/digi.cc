// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Python extension module eicrecon._digi: the digitization factory types
// (src/factories/digi) for eicrecon.factories.digi, and C++ helpers for config values that
// Python cannot write. Add one bind_factory line per factory type that a wiring uses.

#include <pybind11/pybind11.h>

#include "FactoryType.h"
#include "ConfigConversion.h"
#include "SubDivideFunctorBindings.h"
#include "algorithms/tracking/TrackPropagationConfig.h"
#include "factories/digi/CALOROCDigitization_factory.h"
#include "factories/digi/CFDROCDigitization_factory.h"
#include "factories/digi/EICROCDigitization_factory.h"
#include "factories/digi/MPGDTrackerDigi_factory.h"
#include "factories/digi/PhotoMultiplierHitDigi_factory.h"
#include "factories/digi/PulseCombiner_factory.h"
#include "factories/digi/PulseGeneration_factory.h"
#include "factories/digi/PulseNoise_factory.h"
#include "factories/digi/RandomNoisePixel_factory.h"
#include "factories/digi/SiliconChargeSharing_factory.h"
#include "factories/digi/SiliconPulseDiscretization_factory.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "services/geometry/richgeo/ActsGeo.h"
#include "services/geometry/richgeo/RichGeo.h"
#include "services/geometry/richgeo/RichGeo_service.h"
#include <DD4hep/DetElement.h>
#include <DD4hep/Detector.h>
#include <JANA/JApplication.h>
#include <edm4eic/MCRecoTrackParticleAssociation.h>
#include <edm4eic/MCRecoTrackParticleLinkCollection.h>
#include <edm4eic/RawTrackerHit.h>
#include <edm4eic/Track.h>
#include <edm4eic/TrackerHit.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <edm4hep/SimTrackerHit.h>
#include <format> // IWYU pragma: keep
#include <podio/detail/Link.h>
#include <string>

using namespace eicrecon;

namespace py = pybind11;
using eicrecon::python::bind_factory;

PYBIND11_MODULE(_digi, m) {
  m.doc() = "EICrecon digitization factory types";
  py::module_::import("eicrecon._core");
  bind_factory<CALOROCDigitization_factory>(m, "CALOROCDigitization");
  bind_factory<CFDROCDigitization_factory>(m, "CFDROCDigitization");
  bind_factory<EICROCDigitization_factory>(m, "EICROCDigitization");
  bind_factory<MPGDTrackerDigi_factory>(m, "MPGDTrackerDigi");
  bind_factory<PhotoMultiplierHitDigi_factory>(m, "PhotoMultiplierHitDigi");
  bind_factory<PulseCombiner_factory>(m, "PulseCombiner");
  bind_factory<PulseGeneration_factory<edm4hep::SimCalorimeterHit>>(
      m, "PulseGeneration_SimCalorimeterHit");
  bind_factory<PulseGeneration_factory<edm4hep::SimTrackerHit>>(m, "PulseGeneration_SimTrackerHit");
  bind_factory<PulseNoise_factory>(m, "PulseNoise");
  bind_factory<RandomNoisePixel_factory>(m, "RandomNoisePixel");
  bind_factory<SiliconChargeSharing_factory>(m, "SiliconChargeSharing");
  bind_factory<SiliconPulseDiscretization_factory>(m, "SiliconPulseDiscretization");
  bind_factory<SiliconTrackerDigi_factory>(m, "SiliconTrackerDigi");
}
