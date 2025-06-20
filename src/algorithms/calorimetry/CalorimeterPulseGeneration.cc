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
}

void CalorimeterPulseGeneration::process(const CalorimeterPulseGeneration::Input& input,
                                         const CalorimeterPulseGeneration::Output& output) const {

  const auto [simhits] = input;
  auto [simpulses]     = output;

  for (const auto& sh : *simhits) {
    double charge = sh.getEnergy();

    // find the earliest time
    auto contribs = sh.getContributions();
    auto earTimeContrib =
        std::min_element(contribs.begin(), contribs.end(),
                         [](const auto& a, const auto& b) { return a.getTime() < b.getTime(); });
    auto earTime = (*earTimeContrib).getTime();

    double signal_time = m_cfg.timestep * std::floor(earTime / m_cfg.timestep);

    bool passed_threshold = false;
    int skip_bins         = 0;
    float integral        = 0;
    std::vector<float> pulseAmplitudes;

    for (int i = 0; i < m_cfg.max_time_bin; i++) {
      double t    = signal_time + i * m_cfg.timestep - earTime;
      auto signal = (*m_pulse)(t, charge);

      if (std::abs(signal) < m_cfg.ignore_thres) {
        if (passed_threshold == false) {
          skip_bins = i;
          continue;
        }
        if (t > m_min_sampling_time) {
          break;
        }
      }

      passed_threshold = true;
      pulseAmplitudes.push_back(signal);
      integral += signal;
    }

    auto pulse = simpulses->create();
    pulse.setCellID(sh.getCellID());
    pulse.setInterval(m_cfg.timestep);
    pulse.setTime(earTime);

    for (const auto& pulseAmplitude : pulseAmplitudes) {
      pulse.addToAmplitude(pulseAmplitude);
    }

#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
    pulse.setIntegral(integral);
    pulse.setPosition(sh.getPosition());
    pulse.addToCalorimeterHits(sh);
    pulse.addToParticles(sh.getContributions(0).getParticle());
#endif
  }
}

} // namespace eicrecon
