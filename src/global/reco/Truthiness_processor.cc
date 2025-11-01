// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#include "Truthiness_processor.h"

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/JEvent.h>
#include <algorithms/logger.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <map>
#include <string>

#include "services/log/Log_service.h"

namespace eicrecon {

void Truthiness_processor::Init() {
  std::string plugin_name = "Truthiness";
  auto* app               = GetApplication();

  // Get logger
  m_log = app->GetService<Log_service>()->logger(plugin_name);

  // Initialize algorithm
  m_algo = std::make_unique<Truthiness>(plugin_name);
  // Set algorithm log level to match processor log level
  m_algo->level(static_cast<algorithms::LogLevel>(m_log->level()));
  m_algo->init();

  // Get input collection names from parameters
  app->SetDefaultParameter(plugin_name + ":inputMCParticles", m_inputMCParticles,
                           "Name of input MC particles collection");
  app->SetDefaultParameter(plugin_name + ":inputReconstructedParticles",
                           m_inputReconstructedParticles,
                           "Name of input reconstructed particles collection");
  app->SetDefaultParameter(plugin_name + ":inputAssociations", m_inputAssociations,
                           "Name of input MC-reco associations collection");

  m_log->info("Initialized with collections: MC='{}', Reco='{}', Assoc='{}'", m_inputMCParticles,
              m_inputReconstructedParticles, m_inputAssociations);
}

void Truthiness_processor::Process(const std::shared_ptr<const JEvent>& event) {
  // Get input collections
  const auto* mc_particles = event->GetCollection<edm4hep::MCParticle>(m_inputMCParticles);
  const auto* rc_particles =
      event->GetCollection<edm4eic::ReconstructedParticle>(m_inputReconstructedParticles);
  const auto* associations =
      event->GetCollection<edm4eic::MCRecoParticleAssociation>(m_inputAssociations);

  if (!mc_particles || !rc_particles || !associations) {
    m_log->debug("Event {}: Missing required collections", event->GetEventNumber());
    return;
  }

  m_log->debug("Event {}: Processing {} MC particles, {} reco particles, {} associations",
               event->GetEventNumber(), mc_particles->size(), rc_particles->size(),
               associations->size());

  // Call the algorithm
  Truthiness::Input input{mc_particles, rc_particles, associations};
  Truthiness::Output output{};
  m_algo->process(input, output);
}

void Truthiness_processor::Finish() {
  if (m_algo) {
    const auto event_count = m_algo->getEventCount();
    if (event_count > 0) {
      const auto average_truthiness = m_algo->getAverageTruthiness();
      m_log->info("Processed {} events, average truthiness: {:.6f}", event_count,
                  average_truthiness);
    } else {
      m_log->info("No events processed");
    }
  }
}

} // namespace eicrecon
