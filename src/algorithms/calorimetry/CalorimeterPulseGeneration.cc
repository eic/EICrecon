// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim

#include "CalorimeterPulseGeneration.h"

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <DD4hep/config.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <Evaluator/DD4hepUnits.h>
#include <algorithms/service.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/unit_system.h>
#include <fmt/core.h>
#include <podio/RelationRange.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <gsl/pointers>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <RtypesCore.h>
#include <TMath.h>
#include <edm4hep/MCParticle.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <functional>
#include <algorithm>

#include "algorithms/calorimetry/CalorimeterPulseGenerationConfig.h"
#include "algorithms/digi/SiliconPulseGeneration.h"
#include "services/evaluator/EvaluatorSvc.h"

namespace eicrecon {

void CalorimeterPulseGeneration::init() {
  // readout checks
  if (m_cfg.readout.empty()) {
    error("readoutClass is not provided, it is needed to know the fields in readout ids");
    throw std::runtime_error("readoutClass is not provided");
  }

  // get pulse
  m_pulse = PulseShapeFactory::createPulseShape(m_cfg.pulse_shape, m_cfg.pulse_shape_params);
  m_min_sampling_time = m_cfg.min_sampling_time;

  if (m_pulse->getMaximumTime() > m_min_sampling_time) {
    m_min_sampling_time = m_pulse->getMaximumTime();
  }

  if (m_cfg.edep_to_npe) {
    m_edep_to_npe = m_cfg.edep_to_npe;
  }

  m_amplitude_thres = m_cfg.ignore_thres;

  // random seed
  m_gen.seed(std::random_device{}());
}

void CalorimeterPulseGeneration::process(const CalorimeterPulseGeneration::Input& input,
                                         const CalorimeterPulseGeneration::Output& output) const {

  const auto [simhits] = input;
  auto [simpulses]     = output;

  for (const auto& sh : *simhits) {
    std::vector<edm4hep::CaloHitContribution> ordered_contribs;

    // fill the contributions in the editable form
    auto contribs = sh.getContributions();
    std::vector<edm4hep::CaloHitContribution> ordered_contribs(contribs.begin(), contribs.end());

    // sort the contributions by time
    std::sort(contribs.begin(), contribs.end(),
              [](const edm4hep::CaloHitContribution& a, const edm4hep::CaloHitContribution& b) {
                return a.getTime() < b.getTime();
              });

    auto earliest_time = contribs.front().getTime();
    auto latest_time   = contribs.back().getTime();
    int max_time_index =
        std::round((earliest_time - latest_time) / m_cfg.timestep) + m_cfg.max_time_bin;
    std::vector<double> amplitudes(max_time_index, 0.);

    int min_time_index_store   = std::numeric_limits<int>::max();
    int max_time_index_store   = 0.;
    double combined_pulse_time = std::numeric_limits<double>::max();

    // build pulses for each contribution and combine them
    for (const auto& contrib : sh.getContributions()) {
      double pulse_height = contrib.getEnergy();
      double hit_time     = contrib.getTime();

      // convert energy deposit to npe and apply poisson smearing ** if necessary **
      if (m_edep_to_npe) {
        double npe = pulse_height * m_edep_to_npe.value();
        std::poisson_distribution<> poisson(npe);
        pulse_height = poisson(m_gen);

        m_amplitude_thres *= m_edep_to_npe.value();
      }

      double signal_time = m_cfg.timestep * std::floor(time / m_cfg.timestep);

      bool passed_threshold = false;
      int skip_bins         = 0;

      for (int i = 0; i < m_cfg.max_time_bin; i++) {
        double t         = i * m_cfg.timestep;
        double amplitude = (*m_pulse)(t, pulse_height);

        int time_index = i + std::round((time - earliest_time) / m_cfg.timestep);

        if (std::abs(amplitude) < m_amplitude_thres) {
          if (passed_threshold == false) {
            skip_bins = i;
            continue;
          }
          if (t > m_min_sampling_time) {
            max_time_index_store = time_index;
            break;
          }
        }

        passed_threshold = true;
        amplitudes[time_index] += pulse_height;
      }

      double pulse_time   = signal_time + skip_bins * m_cfg.timestep;
      combined_pulse_time = std::min(combined_pulse_time, pulse_time);

      double pulse_time_index = skip_bins + std::round((time - earliest_time) / m_cfg.timestep);
      min_time_index_store    = std::min(min_time_index_store, pulse_time_index);
    }

    /*auto pulse = simpulses->create();
                        pulse.setCellID(sh.getCellID());
                        pulse.setInterval(m_cfg.timestep);
                        pulse.setTime(pulse_time);

                        amplitudes.erase(amplitudes.begin() + max_time_index_store, amplitudes.end());
                        amplitudes.erase(amplitudes.begin(), amplitudes.begin() + min_time_index_store);

                        double integral = 0;
                        for (const auto& amplitude : amplitudes) {
                                integral += amplitude;
                                pulse.addToAmplitude(amplitude);
                        }

#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
                        pulse.setIntegral(integral);
                        pulse.setPosition(sh.getPosition());
                        pulse.addToCalorimeterHits(sh);
                        pulse.addToParticles(sh.getContributions(0).getParticle());
#endif*/
  }
}

} // namespace eicrecon
