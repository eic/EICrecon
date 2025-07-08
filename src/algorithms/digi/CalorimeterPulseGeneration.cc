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
  // Initialize seed.
  // To make the algorithm reproducible, a fixed value was used for the seed.
  m_gen.seed(5140);

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

  // To build a pulse for a hit, this algorithm builds pulses for each
  // contribution and combine them.
  for (const auto& sh : *simhits) {
    // Fill the contributions in the editable form.
    auto contribs = sh.getContributions();
    std::vector<edm4hep::CaloHitContribution> ordered_contribs(contribs.begin(), contribs.end());

    // Sort the contributions by time.
    std::sort(ordered_contribs.begin(), ordered_contribs.end(),
              [](const edm4hep::CaloHitContribution& a, const edm4hep::CaloHitContribution& b) {
                return a.getTime() < b.getTime();
              });

    // Group the contributions that are close in time to combine and store them separately
    // because some contributions are significantly separated from the others in time
    std::vector<std::vector<edm4hep::CaloHitContribution>> contrib_clusters;
    for (const auto& contrib : ordered_contribs) {
      if (contrib_clusters.size() == 0)
        contrib_clusters.push_back({contrib});
      else {
        edm4hep::CaloHitContribution last_contrib = contrib_clusters.back().back();
        if (contrib.getTime() - last_contrib.getTime() < m_cfg.minimum_separation) {
          contrib_clusters.back().push_back(contrib);
        } else {
          contrib_clusters.push_back({contrib});
        }
      }
    }

    for (const auto& contribs : contrib_clusters) {
      double earliest_time = contribs.front().getTime();

      // A vector with a temporary size is created to accumulate the amplitudes.
      // The size will increase when necessary
      std::vector<double> amplitudes(m_cfg.max_time_bin, 0.);

      // After accumulating all the amplitudes, the vector size will be reduced
      // so that only the amplitudes greater than m_ignore_thres are stored.
      int min_time_bin_store = std::numeric_limits<int>::max();
      int max_time_bin_store = 0;

      // build pulses for each contribution and combine them.
      for (const auto& contrib : contribs) {
        double pulse_height = contrib.getEnergy();
        double time         = contrib.getTime();

        // convert energy deposit to npe and apply poisson smearing ** if necessary **
        if (m_edep_to_npe) {
          double npe = pulse_height * m_edep_to_npe.value();
          std::poisson_distribution<> poisson(npe);
          pulse_height = poisson(m_gen);
        }

        // if the pulse height is lower than m_ignore_thres, it is not necessary to scan it.
        if ((*m_pulse)(m_pulse->getMaximumTime(), pulse_height) < m_ignore_thres)
          continue;

        bool passed_threshold = false;
        int last_skip_bin     = 0;

        for (std::uint32_t i = 0; i < m_cfg.max_time_bin; i++) {
          double rel_time  = i * m_cfg.timestep;
          double amplitude = (*m_pulse)(rel_time, pulse_height);

          int abs_time_bin = i + std::round((time - earliest_time) / m_cfg.timestep);
          // Increase the vector size if necessary
          if (amplitudes.size() <= abs_time_bin)
            amplitudes.resize(abs_time_bin + m_cfg.max_time_bin);

          // to find the two indices where the pulse meets the m_ignore_thres
          if (std::abs(amplitude) < m_ignore_thres) {
            if (passed_threshold == false) {
              last_skip_bin = i;
              continue;
            }
            if (rel_time > m_min_sampling_time) {
              max_time_bin_store = abs_time_bin;
              break;
            }
          }

          passed_threshold = true;
          amplitudes[abs_time_bin] += pulse_height;
        }

        int pulse_time_bin_contrib =
            last_skip_bin + std::round((time - earliest_time) / m_cfg.timestep);
        min_time_bin_store = std::min(min_time_bin_store, pulse_time_bin_contrib);
      }

      // if all the pulse heights are lower than the m_ignore_thres,
      // it is not necessary to store this hit.
      if (max_time_bin_store == 0)
        continue;

      double pulse_time_hit = earliest_time + min_time_bin_store * m_cfg.timestep;

      auto pulse = simpulses->create();
      pulse.setCellID(sh.getCellID());
      pulse.setInterval(m_cfg.timestep);
      pulse.setTime(pulse_time_hit);

      // reduce the vertor size.
      amplitudes.erase(amplitudes.begin() + max_time_bin_store, amplitudes.end());
      amplitudes.erase(amplitudes.begin(), amplitudes.begin() + min_time_bin_store);

      double integral = 0;
      for (const auto& amplitude : amplitudes) {
        integral += amplitude;
        pulse.addToAmplitude(amplitude);
      }

#if edm4eic_version_major > 8 || (edm4eic_version_major == 8 && edm4eic_version_minor >= 1)
      pulse.setIntegral(integral);
      pulse.setPosition(sh.getPosition());
      pulse.addToCalorimeterhits(sh);
      pulse.addtoparticles(sh.getContributions(0).getParticle());
#endif
    }
  }
}

} // namespace eicrecon
