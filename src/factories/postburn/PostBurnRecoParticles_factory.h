// Created by Alex Jentsch
// Subject to the terms in the LICENSE file found in the top-level directory.
//

//#include "algorithms/fardetectors/MatrixTransferStaticConfig.h"

// Event Model related classes
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>

#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class PostBurnRecoParticles_factory :
    public JOmniFactory<PostBurnRecoParticles_factory> { //, MatrixTransferStaticConfig> {

public:
    using AlgoT = eicrecon::PostBurn;
private:
    std::unique_ptr<AlgoT> m_algo;

    PodioInput<edm4hep::MCParticle> m_mcparts_input {this};
	PodioInput<edm4eic::ReconstructedParticle> m_recoparts_input {this};
	PodioInput<edm4eic::MCRecoParticleAssociation> m_recoparts_assoc_input {this};
    PodioOutput<edm4eic::ReconstructedParticle> m_postburn_output {this};

    ParameterRef<float>     partMass  {this, "partMass", config().partMass};
    ParameterRef<float>     partCharge{this, "partCharge", config().partCharge};
    ParameterRef<long long> partPDG   {this, "partPDG", config().partPDG};

    ParameterRef<double> crossingAngle       {this, "crossingAngle", config().crossingAngle};
    ParameterRef<double> nomMomentum         {this, "nomMomentum", config().nomMomentum};

    // FIXME JANA2 does not support vector of vector
    //ParameterRef<std::vector<std::vector<double>>> aX {this, "aX", config().aX};
    //ParameterRef<std::vector<std::vector<double>>> aY {this, "aY", config().aY};


    ParameterRef<std::string> readout {this, "readout", config().readout};

public:
    void Configure() {
        m_algo = std::make_unique<AlgoT>(GetPrefix());
        m_algo->applyConfig(config());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_algo->process({m_mcparts_input(), m_recoparts_input(), m_recoparts_assoc_input()}, {m_postburn_output().get()});
    }

};

} // eicrecon
