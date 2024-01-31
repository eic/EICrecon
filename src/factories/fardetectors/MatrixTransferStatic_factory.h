// Created by Alex Jentsch
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <DDRec/CellIDPositionConverter.h>
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "algorithms/fardetectors/MatrixTransferStatic.h"
#include "algorithms/fardetectors/MatrixTransferStaticConfig.h"

// Event Model related classes
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>

#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class MatrixTransferStatic_factory :
    public JOmniFactory<MatrixTransferStatic_factory, MatrixTransferStaticConfig> {

public:
    using AlgoT = eicrecon::MatrixTransferStatic;
private:
    std::unique_ptr<AlgoT> m_algo;

    PodioInput<edm4hep::SimTrackerHit> m_hits_input {this};
    PodioOutput<edm4eic::ReconstructedParticle> m_tracks_output {this};

    Service<DD4hep_service> m_geoSvc {this};

    ParameterRef<float>     partMass  {this, "partMass", config().partMass};
    ParameterRef<float>     partCharge{this, "partCharge", config().partCharge};
    ParameterRef<long long> partPDG   {this, "partPDG", config().partPDG};

    ParameterRef<double> local_x_offset      {this, "local_x_offset", config().local_x_offset};
    ParameterRef<double> local_y_offset      {this, "local_y_offset", config().local_y_offset};
    ParameterRef<double> local_x_slope_offset{this, "local_x_slope_offset", config().local_x_slope_offset};
    ParameterRef<double> local_y_slope_offset{this, "local_y_slope_offset", config().local_y_slope_offset};
    ParameterRef<double> crossingAngle       {this, "crossingAngle", config().crossingAngle};
    ParameterRef<double> nomMomentum         {this, "nomMomentum", config().nomMomentum};

    ParameterRef<double> aX00 {this, "aX00", config().aX[0][0]};
    ParameterRef<double> aX01 {this, "aX01", config().aX[0][1]};
    ParameterRef<double> aX10 {this, "aX10", config().aX[1][0]};
    ParameterRef<double> aX11 {this, "aX11", config().aX[1][1]};
    ParameterRef<double> aY00 {this, "aY00", config().aY[0][0]};
    ParameterRef<double> aY01 {this, "aY01", config().aY[0][1]};
    ParameterRef<double> aY10 {this, "aY10", config().aY[1][0]};
    ParameterRef<double> aY11 {this, "aY11", config().aY[1][1]};

    ParameterRef<double> hit1minZ {this, "hit1minZ", config().hit1minZ};
    ParameterRef<double> hit1maxZ {this, "hit1maxZ", config().hit1maxZ};
    ParameterRef<double> hit2minZ {this, "hit2minZ", config().hit2minZ};
    ParameterRef<double> hit2maxZ {this, "hit2maxZ", config().hit2maxZ};

    ParameterRef<std::string> readout {this, "readout", config().readout};

public:
    void Configure() {
        m_algo = std::make_unique<AlgoT>(GetPrefix());
        m_algo->applyConfig(config());
        m_algo->init(m_geoSvc().detector(), logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_algo->process({m_hits_input()}, {m_tracks_output().get()});
    }

};

} // eicrecon
