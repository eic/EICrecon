// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) Derek Anderson

// standard c includes
#include <memory>
#include <JANA/JEvent.h>
// event data model definitions
#include <edm4eic/ReconstructedParticleCollection.h>
// factory-specific includes
#include "ParticleFlow_factory.h"

namespace eicrecon {

  void ParticleFlow_factory::Init() {

    auto app = GetApplication();

    // This prefix will be used for parameters
    std::string plugin_name = GetPluginName();
    std::string param_prefix = plugin_name + ":" + GetTag();

    // SpdlogMixin logger initialization, sets m_log
    InitLogger(GetApplication(), GetPrefix(), "info");

    // Algorithm configuration
    auto cfg = GetDefaultConfig();

    app->SetDefaultParameter(param_prefix + ":flowAlgo",  cfg.flowAlgo);
    app->SetDefaultParameter(param_prefix + ":mergeAlgo", cfg.mergeAlgo);

    // initialize particle flow algorithm
    m_pf_algo.applyConfig(cfg);
    m_pf_algo.init(logger());

  }  // end 'Init()'



  void ParticleFlow_factory::BeginRun(const std::shared_ptr<const JEvent> &event) {

    // Nothing to do here

  }  // end 'BeginRun(std::shared_ptr<JEvent&>)'



  void ParticleFlow_factory::Process(const std::shared_ptr<const JEvent> &event) {

    /* TODO
     *   - organize particle input into tuple
     *   - organize calo into ecal-hcal pairs
     *   - move algo output to output collection
     */

  }  // end 'Process(shared_ptr<JEvent>)'

}  // end eicrecon namespace

