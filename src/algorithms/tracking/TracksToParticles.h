// Original header license: SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Joe Osborn
//

#pragma once


#include <spdlog/logger.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackParametersCollection.h>



namespace eicrecon::Reco {

  //! Converts reconstructed tracks to reconstructed particles
  //! Can add additional detector information when available (e.g. pid, clusters...)
  class TracksToParticles {
  public: 
 
    void init(std::shared_ptr<spdlog::logger> log);
    std::unique_ptr<edm4eic::ReconstructedParticleCollection>
      execute(const std::vector<const edm4eic::Track*>& tracks);

  private:
    std::shared_ptr<spdlog::logger> m_log;
  };


}
