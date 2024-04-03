// Copyright 2024, Nathan Brei
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include "services/pid_lut/PIDLookupTable_service.h"
#include "extensions/jana/JOmniFactory.h"


namespace eicrecon {

class PIDLookupTable_factory : public JOmniFactory<PIDLookupTable_factory> {

private:
    PodioInput<edm4eic::???> m_particle_input {this};
    PodioOutput<edm4eic::???> m_particle_id_output {this};

    Parameter<std::string> m_filename {this, "filename"};
    Service<PIDLookupTable_service> m_lut_svc {this};
    PIDLookupTable* m_lut;

public:
    void Configure() {
        m_lut = m_lut_svc()->FetchTable(m_filename());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {

        // TODO: What are our data model inputs and outputs?
    }
};

} // eicrecon
