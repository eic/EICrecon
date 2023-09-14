// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Simon Gardner

#pragma once

#include <spdlog/logger.h>
#include <edm4hep/MCParticleCollection.h>
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"
#include "algorithms/reco/MCParticleIsolator.h"

namespace eicrecon {

/// This factory filters an MCParticles collection by pdg and genstatus
class MCParticleIsolator_factory :
    public JChainMultifactoryT<MCParticleIsolatorConfig>,
    public SpdlogMixin {

  public:
    explicit MCParticleIsolator_factory(
        std::string tag,
        const std::vector<std::string>& input_tags,
        const std::vector<std::string>& output_tags,
        MCParticleIsolatorConfig cfg)
      : JChainMultifactoryT<MCParticleIsolatorConfig>(std::move(tag), input_tags, output_tags, cfg) {

      bool owns_data = true; // this produces a subset collection
      DeclarePodioOutput<edm4hep::MCParticle>(GetOutputTags()[0], owns_data);

    }

    /** One time initialization **/
    void Init() override {
      auto app = GetApplication();

      // SpdlogMixin logger initialization, sets m_log
      InitLogger(app, GetPrefix(), "info");

      auto cfg = GetDefaultConfig();

      m_algo.applyConfig(cfg);

      m_algo.init(logger());
    }

    /** Event by event processing **/
    void Process(const std::shared_ptr<const JEvent> &event) override {

      auto particle_collection = static_cast<const edm4hep::MCParticleCollection*>(event->GetCollectionBase(GetInputTags()[0]));

      try {
        auto particles = m_algo.process(*particle_collection);
        SetCollection<edm4hep::MCParticle>(GetOutputTags()[0], std::move(particles));
      }
      catch(std::exception &e) {
        throw JException(e.what());
      }
    }

  private:
    MCParticleIsolator m_algo;

  };

} // eicrecon
