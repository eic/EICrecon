// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <spdlog/spdlog.h>
#include "extensions/spdlog/SpdlogMixin.h"
#include <services/io/podio/JFactoryPodioT.h>
#include <algorithms/digi/SmearedFarForwardParticles.h>


namespace eicrecon {

class ReconstructedParticle_factory_SmearedFarForwardParticles : public eicrecon::JFactoryPodioT<edm4eic::ReconstructedParticle>, public eicrecon::SpdlogMixin<ReconstructedParticle_factory_SmearedFarForwardParticles>, SmearedFarForwardParticles {

public:

    std::string m_input_tag = "MCParticles";

    ReconstructedParticle_factory_SmearedFarForwardParticles() {
        SetTag("SmearedFarForwardParticles");
    };

    /** One time initialization **/
    void Init() override{

        InitLogger("Digi:SmearedFarForwardParticles", "info");
        // (this line seems quite awkward)
        this->SmearedFarForwardParticles::m_log = this->eicrecon::SpdlogMixin<ReconstructedParticle_factory_SmearedFarForwardParticles>::m_log;

        initialize();
    }

    /** Event by event processing **/
    void Process(const std::shared_ptr<const JEvent> &event) override {

        m_inputMCParticles = event->Get<edm4hep::MCParticle>(m_input_tag);

        execute();

        Set( m_outputParticles );
        event->Insert( m_outputAssoc );
        m_outputParticles.clear();  // JANA now owns objects
        m_outputAssoc.clear();      // JANA now owns objects
    }

private:

    std::shared_ptr<spdlog::logger> m_log;              /// Logger for this factory

    int m_verbose;                                      /// verbosity 0-none, 1-default, 2-debug, 3-trace
    std::vector<std::string> m_input_tags;              /// Tag for the input data


};

} // eicrecon
