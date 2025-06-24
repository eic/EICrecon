// Created by Alex Jentsch
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <DDRec/CellIDPositionConverter.h>
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "algorithms/fardetectors/MatrixTransferStatic.h"
#include "algorithms/fardetectors/MatrixTransferStaticConfig.h"

// Event Model related classes
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/MCParticleCollection.h>

#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class MatrixTransferStatic_factory
    : public JOmniFactory<MatrixTransferStatic_factory, MatrixTransferStaticConfig> {

public:
  using AlgoT = eicrecon::MatrixTransferStatic;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::MCParticle> m_mcparts_input{this};
  PodioInput<edm4eic::TrackerHit> m_hits_input{this};
  PodioOutput<edm4eic::ReconstructedParticle> m_tracks_output{this};

  Service<DD4hep_service> m_geoSvc{this};

  ParameterRef<float> partMass{this, "partMass", config().partMass};
  ParameterRef<float> partCharge{this, "partCharge", config().partCharge};
  ParameterRef<long long> partPDG{this, "partPDG", config().partPDG};

  ParameterRef<double> crossingAngle{this, "crossingAngle", config().crossingAngle};

  // FIXME JANA2 does not support vector of vector
  //ParameterRef<std::vector<std::vector<double>>> aX {this, "aX", config().aX};
  //ParameterRef<std::vector<std::vector<double>>> aY {this, "aY", config().aY};

  ParameterRef<double> hit1minZ{this, "hit1minZ", config().hit1minZ};
  ParameterRef<double> hit1maxZ{this, "hit1maxZ", config().hit1maxZ};
  ParameterRef<double> hit2minZ{this, "hit2minZ", config().hit2minZ};
  ParameterRef<double> hit2maxZ{this, "hit2maxZ", config().hit2maxZ};

  ParameterRef<std::string> readout{this, "readout", config().readout};

  ParameterRef<bool> requireBeamProton{this, "requireBeamProton", config().requireBeamProton};
  ParameterRef<bool> requireMatchingMatrix{this, "requireMatchingMatrix",
                                           config().requireMatchingMatrix};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void ChangeRun(int32_t /* run_number */) {}

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_mcparts_input(), m_hits_input()}, {m_tracks_output().get()});
  }
};

} // namespace eicrecon
