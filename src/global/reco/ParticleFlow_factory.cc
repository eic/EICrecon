// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) Derek Anderson

#include <memory>
#include <exception>
#include <stdexcept>
#include <JANA/JEvent.h>
#include <JANA/JException.h>
// event data model definitions
#include <edm4eic/ClusterCollection.h>
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

    // for calo inputs
    std::vector<const edm4eic::ClusterCollection*> vecECalInput(m_const.nCaloPairs);
    std::vector<const edm4eic::ClusterCollection*> vecHCalInput(m_const.nCaloPairs);

    // grab and organize input collections
    TrkInput trkProjectInput;
    for (const std::string& input_tag : GetInputTags()) {

      // check if input is a track projection collection
      if (input_tag.find("TrackProjection") != std::string::npos) {
        trkProjectInput = static_cast<const edm4eic::TrackSegmentCollection*>(event->GetCollectionBase(input_tag));
        m_log->debug("Found input track projection collection {}", input_tag);
        continue;
      }

      // check if input is a calo collection
      bool isECalCollect = (input_tag.find("Ecal")     != std::string::npos);
      bool isHCalCollect = (input_tag.find("Hcal")     != std::string::npos);
      bool endsInClust   = (input_tag.find("Clusters") != std::string::npos);
      if ((isECalCollect || isHCalCollect) && endsInClust) {

        // set location in input vectors
        size_t iInput = -1;
        try {
          iInput = m_mapCaloInputToIndex[input_tag];
        } catch (std::out_of_range &out) {
          throw JException(out.what());
        }
        m_log->debug("Assigned input calo collection {} an index of {}", input_tag, iInput);

        // grab collections
        if (isECalCollect) vecECalInput[iInput] = static_cast<const edm4eic::ClusterCollection*>(event->GetCollectionBase(input_tag));
        if (isHCalCollect) vecHCalInput[iInput] = static_cast<const edm4eic::ClusterCollection*>(event->GetCollectionBase(input_tag));
      }
    }  // end input tag loop

    // collect input calo collections into ECal-HCal pairs
    std::vector<CaloInput> vecCaloInput(m_const.nCaloPairs);
    for (size_t iCaloPair = 0; iCaloPair < m_const.nCaloPairs; iCaloPair++) {
      vecCaloInput[iCaloPair] = std::make_pair(vecECalInput[iCaloPair], vecHCalInput[iCaloPair]);
    }

    // run algorithm
    auto pf_objects = m_pf_algo.process(trkProjectInput, vecCaloInput);

    // set output collection
    SetCollection<edm4eic::ReconstructedParticle>(GetOutputTags()[0], std::move(pf_objects));

  }  // end 'Process(shared_ptr<JEvent>)'

}  // end eicrecon namespace

