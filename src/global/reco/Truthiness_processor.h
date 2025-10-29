// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#include <JANA/JEventProcessor.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <memory>
#include <string>
#include <spdlog/logger.h>

#include "algorithms/reco/Truthiness.h"

namespace eicrecon {

class Truthiness_processor : public JEventProcessor {
private:
  std::unique_ptr<Truthiness> m_algo;
  std::shared_ptr<spdlog::logger> m_log;
  std::string m_inputMCParticles{"MCParticles"};
  std::string m_inputReconstructedParticles{"ReconstructedParticles"};
  std::string m_inputAssociations{"ReconstructedParticleAssociations"};

public:
  Truthiness_processor() { SetTypeName(NAME_OF_THIS); }

  void Init() override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void Finish() override;
};

} // namespace eicrecon
