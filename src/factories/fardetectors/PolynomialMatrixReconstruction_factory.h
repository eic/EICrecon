// Created by Alex Jentsch
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <DDRec/CellIDPositionConverter.h>
#include "algorithms/fardetectors/PolynomialMatrixReconstruction.h"
#include "algorithms/fardetectors/PolynomialMatrixReconstructionConfig.h"

// Event Model related classes
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/MCParticleCollection.h>

#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class PolynomialMatrixReconstruction_factory
    : public JOmniAlgorithmFactory<PolynomialMatrixReconstruction_factory,
                                   eicrecon::PolynomialMatrixReconstruction,
                                   PolynomialMatrixReconstructionConfig> {
private:
  Service<DD4hep_service> m_geoSvc{this};

  ParameterRef<float> partMass{this, "partMass", config().partMass};
  ParameterRef<float> partCharge{this, "partCharge", config().partCharge};
  ParameterRef<long long> partPDG{this, "partPDG", config().partPDG};

  ParameterRef<double> crossingAngle{this, "crossingAngle", config().crossingAngle};

  ParameterRef<double> hit1minZ{this, "hit1minZ", config().hit1minZ};
  ParameterRef<double> hit1maxZ{this, "hit1maxZ", config().hit1maxZ};
  ParameterRef<double> hit2minZ{this, "hit2minZ", config().hit2minZ};
  ParameterRef<double> hit2maxZ{this, "hit2maxZ", config().hit2maxZ};

  ParameterRef<std::string> readout{this, "readout", config().readout};

  ParameterRef<bool> requireBeamProton{this, "requireBeamProton", config().requireBeamProton};
};

} // namespace eicrecon
