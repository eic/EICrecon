// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Simon Gardner

#pragma once

#include <spdlog/spdlog.h>


// Event Model related classes
#include <edm4hep/MCParticleCollection.h>

#include "algorithms/interfaces/WithPodConfig.h"
#include "MCParticleIsolatorConfig.h"

namespace eicrecon {

  /** Selects MC particles with only.
   *
   * \ingroup reco
   */
  class MCParticleIsolator : public WithPodConfig<MCParticleIsolatorConfig> {

  public:

    void init(std::shared_ptr<spdlog::logger>& logger) {
        m_log = logger;
    }

    
    std::unique_ptr<edm4hep::MCParticleCollection> process(
        const edm4hep::MCParticleCollection& part_collection
    ) {

      auto parts = std::make_unique<edm4hep::MCParticleCollection>();
      //parts->setSubsetCollection();

      for (const auto& part : part_collection) {
/* 	std::cout << part.getGeneratorStatus() << " " << m_cfg.genStatus << std::endl; */
/* 	std::cout << part.getPDG() << " " << m_cfg.pdg << std::endl; */
	if(part.getGeneratorStatus()==m_cfg.genStatus && part.getPDG()==m_cfg.pdg){
/* 	  std::cout << "GOOD" << std::endl; */
	  parts->push_back(part);
	}
      }

      return std::move(parts);
    }

  protected:

    std::shared_ptr<spdlog::logger> m_log;

  };

} // namespace eicrecon
