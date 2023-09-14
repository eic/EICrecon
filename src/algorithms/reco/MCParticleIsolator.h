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
        if(part.getGeneratorStatus()==m_cfg.genStatus){
          if(m_cfg.abovePDG){
            if(part.getPDG()>=m_cfg.pdg){
              parts->push_back(part.clone());
            }
          }
          else{
            if(part.getPDG()==m_cfg.pdg){
              parts->push_back(part.clone());
            }
          }
        }
      }

      return std::move(parts);
    }

  protected:

    std::shared_ptr<spdlog::logger> m_log;

  };

} // namespace eicrecon
