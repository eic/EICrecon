// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck, Dmitry Kalinkin

#include <vector>

#include <spdlog/logger.h>
#include <edm4hep/MCParticle.h>

#include "algorithms/reco/ChargedParticleSelector.h"
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"

namespace eicrecon {

/// This factory selects a subset of particles that are charged
class ChargedParticleSelector_factory :
    public JChainMultifactoryT<NoConfig>,
    public SpdlogMixin {

  public:
    explicit ChargedParticleSelector_factory(
        std::string tag,
        const std::vector<std::string>& input_tags,
        const std::vector<std::string>& output_tags)
    : JChainMultifactoryT<NoConfig>(std::move(tag), input_tags, output_tags) {

      bool owns_data = false; // this produces a subset collection
      DeclarePodioOutput<edm4hep::MCParticle>(GetOutputTags()[0], owns_data);

    }

    /** One time initialization **/
    void Init() override {
      auto app = GetApplication();

      // SpdlogMixin logger initialization, sets m_log
      InitLogger(app, GetPrefix(), "info");

      m_algo.init(logger());
    }

    /** Event by event processing **/
    void Process(const std::shared_ptr<const JEvent> &event) override {
      auto particles = event->GetCollection<edm4hep::MCParticle>(GetInputTags()[0]);

      try {
        auto charged_particles = m_algo.process(*particles);
        SetCollection<edm4hep::MCParticle>(GetOutputTags()[0], std::move(charged_particles));
      }
      catch(std::exception &e) {
        throw JException(e.what());
      }
    }

  private:
    ChargedParticleSelector m_algo;

  };

}
