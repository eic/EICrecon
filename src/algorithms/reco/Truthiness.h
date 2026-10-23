// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <atomic>
#include <mutex>
#include <stdint.h>
#include <string>
#include <string_view>
#if __has_include(<edm4eic/Truthiness.h>)
#include <edm4eic/TruthinessCollection.h>
#endif

#include "TruthinessConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

#if __has_include(<edm4eic/Truthiness.h>)
using TruthinessAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::MCParticleCollection, edm4eic::ReconstructedParticleCollection,
                      edm4eic::MCRecoParticleAssociationCollection>,
    algorithms::Output<edm4eic::TruthinessCollection>>;
#else
using TruthinessAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::MCParticleCollection, edm4eic::ReconstructedParticleCollection,
                      edm4eic::MCRecoParticleAssociationCollection>,
    algorithms::Output<>>;
#endif

/**
 * Algorithm computing a "truthiness" metric that quantifies how well reconstructed
 * particles agree with the Monte Carlo (MC) truth information.
 *
 * The truthiness value is a scalar penalty score built from several contributions,
 * such as:
 *   - energy penalties for mismatches between reconstructed and MC energies,
 *   - momentum penalties for discrepancies in reconstructed vs MC momentum,
 *   - penalties for PID (particle identification) mismatches between reco and MC,
 *   - penalties for unassociated reconstructed particles or MC particles.
 *
 * These contributions are combined into a single number per event (and/or per
 * object, depending on configuration). Lower truthiness values indicate better
 * agreement between reconstruction and truth, while higher values indicate larger
 * or more confident disagreement — i.e. "more truthiness is more confidently
 * wrong". This class also keeps simple running statistics (average truthiness and
 * number of processed events) for monitoring purposes.
 */
class Truthiness : public TruthinessAlgorithm, public WithPodConfig<TruthinessConfig> {

private:
  mutable double m_average_truthiness{0.0};
  mutable std::atomic<uint64_t> m_event_count{0};
  mutable std::mutex m_stats_mutex;

public:
  Truthiness(std::string_view name)
      : TruthinessAlgorithm{
            name,
            {"inputMCParticles", "inputReconstructedParticles", "inputAssociations"},
#if __has_include(<edm4eic/Truthiness.h>)
            {"outputTruthiness"},
#else
            {},
#endif
            "Calculate truthiness metric comparing reconstructed particles to MC "
            "truth."} {
  }

  void init() final {}
  void process(const Input&, const Output&) const final;

  // Accessors for statistics
  double getAverageTruthiness() const {
    std::lock_guard<std::mutex> lock(m_stats_mutex);
    return m_average_truthiness;
  }
  uint64_t getEventCount() const { return m_event_count.load(); }
};

} // namespace eicrecon
