// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Aiden Wu
// Convert RawCALOROCHits into RawCalorimeterHits for downstream reconstruction.

#include "CALOROCToRawCalorimeterHit.h"

#include <Evaluator/DD4hepUnits.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>

namespace eicrecon {

void CALOROCToRawCalorimeterHit::init() {
  if (m_cfg.calorocType != "1A" && m_cfg.calorocType != "1B") {
    throw std::runtime_error("calorocType must be 1A or 1B");
  }

  m_stepTDC = dd4hep::ns / m_cfg.resolutionTDC;
}

void CALOROCToRawCalorimeterHit::process(const CALOROCToRawCalorimeterHit::Input& input,
                                        const CALOROCToRawCalorimeterHit::Output& output) const {
  const auto [caloroc_hits, pulses] = input;
  auto [rawhits, links, rawassocs]  = output;

  // Loop over digitized channels.
  for (std::size_t hit_index = 0; hit_index < caloroc_hits->size(); hit_index++) {
    const auto caloroc_hit = (*caloroc_hits)[hit_index];

    double response           = 0;
    double time               = 0;
    bool found_time           = false;
    const double sample_phase = caloroc_hit.getSamplePhase() /
                                static_cast<double>(m_cfg.caloroc.capTOA) *
                                m_cfg.caloroc.dyRangeTOA;
    const std::size_t sample_count = m_cfg.calorocType == "1A"
                                         ? caloroc_hit.getASamples().size()
                                         : caloroc_hit.getBSamples().size();

    // Sum the ADC readings and decode the first available arrival time.
    // Placeholder logic.
    for (std::size_t sample_index = 0; sample_index < sample_count; sample_index++) {
      unsigned int toa = 0;
      if (m_cfg.calorocType == "1A") {
        const auto sample = caloroc_hit.getASamples(sample_index);
        response += sample.ADC / static_cast<double>(m_cfg.caloroc.capADC) *
                    m_cfg.caloroc.dyRangeSingleGainADC;
        toa = sample.timeOfArrival;
      } else {
        const auto sample = caloroc_hit.getBSamples(sample_index);
        if (sample.highGainADC < m_cfg.caloroc.capADC - 1) {
          response += sample.highGainADC / static_cast<double>(m_cfg.caloroc.capADC) *
                      m_cfg.caloroc.dyRangeHighGainADC;
        } else {
          response += sample.lowGainADC / static_cast<double>(m_cfg.caloroc.capADC) *
                      m_cfg.caloroc.dyRangeLowGainADC;
        }
        toa = sample.timeOfArrival;
      }

      if (!found_time && toa > 0) {
        time = sample_phase +
               (caloroc_hit.getTimeStamp() + sample_index) * m_cfg.caloroc.time_window -
               toa / static_cast<double>(m_cfg.caloroc.capTOA) * m_cfg.caloroc.dyRangeTOA;
        found_time = true;
      }
    }

    const double energy = response * m_cfg.calorocResponseToEnergy;
    const auto pulse = (*pulses)[hit_index];
    if (!found_time) {
      time = pulse.getTime();
    }

    // Encode calibrated energy and time for the established calorimeter reconstruction.
    auto rawhit = rawhits->create();
    rawhit.setCellID(caloroc_hit.getCellID());
    rawhit.setAmplitude(std::clamp(
        std::llround(m_cfg.pedMeanADC + energy / m_cfg.dyRangeADC * m_cfg.capADC), 0LL,
        static_cast<long long>(m_cfg.capADC)));
    rawhit.setTimeStamp(std::max(std::llround(time * m_stepTDC), 0LL));

    // Restore truth relations from the pulse that produced this CALOROC hit.
    double total_response = 0;
    for (const auto hit : pulse.getCalorimeterHits()) {
      total_response += hit.getEnergy();
    }
    if (total_response <= 0) {
      continue;
    }

    for (const auto hit : pulse.getCalorimeterHits()) {
      const double weight = hit.getEnergy() / total_response;

      auto link = links->create();
      link.setFrom(rawhit);
      link.setTo(hit);
      link.setWeight(weight);

      auto association = rawassocs->create();
      association.setRawHit(rawhit);
      association.setSimHit(hit);
      association.setWeight(weight);
    }
  }
}

} // namespace eicrecon
