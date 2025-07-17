// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025 Simon Gardner, Chun Yuen Tsang, Prithwish Tribedy, Minho Kim

#include "CalorimeterPulseGeneration.h"

#include <edm4eic/EDM4eicVersion.h>
#include <edm4hep/CaloHitContribution.h>
#include <edm4hep/MCParticle.h>
#include <podio/RelationRange.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <gsl/pointers>
#include <limits>
#include <vector>

#include "algorithms/digi/CalorimeterPulseGenerationConfig.h"
#include "algorithms/digi/SiliconPulseGeneration.h"

namespace eicrecon {

void CalorimeterPulseGeneration::init() {
  // Get pulse and related parameters
  m_pulse = PulseShapeFactory::createPulseShape(m_cfg.pulse_shape, m_cfg.pulse_shape_params);
  m_min_sampling_time = m_cfg.min_sampling_time;

  if (m_pulse->getMaximumTime() > m_min_sampling_time) {
    m_min_sampling_time = m_pulse->getMaximumTime();
  }

  m_ignore_thres = m_cfg.ignore_thres;

  if (m_cfg.edep_to_npe) {
    m_edep_to_npe = m_cfg.edep_to_npe;
    m_ignore_thres *= m_edep_to_npe.value();
  }
}

void CalorimeterPulseGeneration::process(const CalorimeterPulseGeneration::Input& input,
                                         const CalorimeterPulseGeneration::Output& output) const {

  const auto [simhits] = input;
  auto [simpulses]     = output;

  // To build a pulse for a hit, this algorithm groups contributions
  // that are close in time and builds pulse from each of them.
  for (const auto& sh : *simhits) {
    // Fill the contributions to be sorted by time
    std::vector<const edm4hep::CaloHitContribution*> contribs;
    for (const auto& contrib : sh.getContributions()) {
      contribs.push_back(&contrib);
    }

    // Sort the contributions by time.
    std::sort(contribs.begin(), contribs.end(),
              [](const auto* a, const auto* b) { return a->getTime() < b->getTime(); });

    // Group the contributions that are close in time.
    std::vector<std::vector<const edm4hep::CaloHitContribution*>> contrib_clusters;
    for (const auto& contrib : contribs) {
      if (contrib_clusters.empty())
        contrib_clusters.push_back({contrib});
      else {
        const auto* last_contrib = contrib_clusters.back().back();
        if (contrib->getTime() - last_contrib->getTime() < m_cfg.minimum_separation) {
          contrib_clusters.back().push_back(contrib);
        } else {
          contrib_clusters.push_back({contrib});
        }
      }
    }

    for (const auto& contribs : contrib_clusters) {
      double time = contribs.front()->getTime();
      double pulse_height =
          std::accumulate(contribs.begin(), contribs.end(), 0.0,
                          [](double sum, const edm4hep::CaloHitContribution* contrib) {
                            return sum + contrib->getEnergy();
                          });

      // Convert energy deposit to npe and apply poisson smearing ** if necessary **
      if (m_edep_to_npe) {
        double npe = pulse_height * m_edep_to_npe.value();
        std::poisson_distribution<> poisson(npe);
        pulse_height = poisson(m_gen);
      }

      // If the pulse height is lower than m_ignore_thres, it is not necessary to scan it.
      if ((*m_pulse)(m_pulse->getMaximumTime(), pulse_height) < m_ignore_thres)
        continue;

      double signal_time = m_cfg.timestep * std::floor(time / m_cfg.timestep);

      bool passed_threshold   = false;
      std::uint32_t skip_bins = 0;
      float integral          = 0;
      std::vector<float> amplitudes;

      // Build pulse and scanning the amplitudes.
      for (std::uint32_t i = 0; i < m_cfg.max_time_bins; i++) {
        double t       = signal_time + i * m_cfg.timestep - time;
        auto amplitude = (*m_pulse)(t, pulse_height);
        if (std::abs(amplitude) < m_cfg.ignore_thres) {
          if (!passed_threshold) {
            skip_bins = i;
            continue;
          }
          if (t > m_min_sampling_time) {
            break;
          }
        }
        passed_threshold = true;
        amplitudes.push_back(amplitude);
        integral += amplitude;
      }

      if (!passed_threshold) {
        continue;
      }

      auto pulse = simpulses->create();
      pulse.setCellID(sh.getCellID());
      pulse.setInterval(m_cfg.timestep);
      pulse.setTime(signal_time + skip_bins * m_cfg.timestep);

      for (const auto& amplitude : amplitudes) {
        pulse.addToAmplitude(amplitude);
      }

#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
      pulse.setIntegral(integral);
      pulse.setPosition(sh.getPosition());
      pulse.addToCalorimeterHits(sh);
      pulse.addToParticles(sh.getContributions(0).getParticle());
#endif
    }
  }
}

} // namespace eicrecon
