// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Python binding module eicrecon._wiring_BTOF: the factory types (and C++ helpers) used
// by the Python wiring of the BTOF plugin, src/python/eicrecon/plugins/BTOF.py.
// Add one bind_factory line per factory type that the wiring uses.

#include <edm4hep/SimTrackerHit.h>
#include <pybind11/pybind11.h>
#include <string>

#include "FactoryType.h"
#include "factories/digi/CFDROCDigitization_factory.h"
#include "factories/digi/PulseCombiner_factory.h"
#include "factories/digi/PulseGeneration_factory.h"
#include "factories/digi/SiliconChargeSharing_factory.h"
#include "factories/digi/SiliconPulseDiscretization_factory.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/reco/LGADHitCalibration_factory.h"
#include "factories/tracking/LGADHitClustering_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"

using namespace eicrecon;

namespace py = pybind11;
using eicrecon::python::bind_factory;

PYBIND11_MODULE(_wiring_BTOF, m) {
  m.doc() = "Factory types wired by the BTOF plugin";
  py::module_::import("eicrecon._core");
  bind_factory<LGADHitCalibration_factory>(m, "LGADHitCalibration");
  bind_factory<LGADHitClustering_factory>(m, "LGADHitClustering");
  bind_factory<SiliconChargeSharing_factory>(m, "SiliconChargeSharing");
  bind_factory<SiliconTrackerDigi_factory>(m, "SiliconTrackerDigi");
  bind_factory<TrackerHitReconstruction_factory>(m, "TrackerHitReconstruction");
  bind_factory<PulseGeneration_factory<edm4hep::SimTrackerHit>>(m, "PulseGeneration_SimTrackerHit");
  bind_factory<PulseCombiner_factory>(m, "PulseCombiner");
  bind_factory<SiliconPulseDiscretization_factory>(m, "SiliconPulseDiscretization");
  bind_factory<CFDROCDigitization_factory>(m, "CFDROCDigitization");
}
