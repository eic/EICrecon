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

  // Use the configured saturation point or the last available ADC count.
  const auto adc_saturation =
      m_cfg.calorocADCSaturation > 0 ? m_cfg.calorocADCSaturation : m_cfg.caloroc.capADC - 1;

  // Calibrate every digitized channel into the legacy raw-hit representation.
  for (std::size_t hit_index = 0; hit_index < caloroc_hits->size(); hit_index++) {
    const auto caloroc_hit = (*caloroc_hits)[hit_index];

    // Accumulate calibrated energy and recover the first valid arrival time.
    double energy             = 0;
    double time               = 0;
    bool found_time           = false;
    const double sample_phase = caloroc_hit.getSamplePhase() /
                                static_cast<double>(m_cfg.caloroc.capTOA) *
                                m_cfg.caloroc.dyRangeTOA;

    // Sum all charge samples for type 1A.
    if (m_cfg.calorocType == "1A") {
      for (std::size_t sample_index = 0; sample_index < caloroc_hit.getASamples().size();
           sample_index++) {
        const auto sample = caloroc_hit.getASamples(sample_index);

        // Convert ADC response into energy across its valid range.
        const double response = sample.ADC / static_cast<double>(m_cfg.caloroc.capADC) *
                                m_cfg.caloroc.dyRangeSingleGainADC;
        double sample_energy = response * m_cfg.calorocResponseToEnergy;

        // Replace saturated ADC response with the calibrated ToT estimate.
        if (sample.ADC >= adc_saturation && sample.timeOverThreshold > 0) {
          const double tot = sample.timeOverThreshold /
                             static_cast<double>(m_cfg.caloroc.capTOT) *
                             m_cfg.caloroc.dyRangeTOT;
          sample_energy = std::max(0.0, tot - m_cfg.calorocTOTOffset) *
                          m_cfg.calorocTOTToEnergy;
        }
        energy += sample_energy;

        // Decode the first available A-sample ToA.
        if (!found_time && sample.timeOfArrival > 0) {
          time = sample_phase +
                 (caloroc_hit.getTimeStamp() + sample_index) * m_cfg.caloroc.time_window -
                 sample.timeOfArrival / static_cast<double>(m_cfg.caloroc.capTOA) *
                     m_cfg.caloroc.dyRangeTOA;
          found_time = true;
        }
      }
    } else {
      // Sum all charge samples for type 1B.
      for (std::size_t sample_index = 0; sample_index < caloroc_hit.getBSamples().size();
           sample_index++) {
        const auto sample = caloroc_hit.getBSamples(sample_index);

        // Switch to low gain if the high-gain ADC reaches saturation.
        const bool high_gain_saturated = sample.highGainADC >= adc_saturation;
        const double response =
            high_gain_saturated
                ? sample.lowGainADC / static_cast<double>(m_cfg.caloroc.capADC) *
                      m_cfg.caloroc.dyRangeLowGainADC
                : sample.highGainADC / static_cast<double>(m_cfg.caloroc.capADC) *
                      m_cfg.caloroc.dyRangeHighGainADC;
        energy += response * m_cfg.calorocResponseToEnergy;

        // Decode the first available B-sample ToA.
        if (!found_time && sample.timeOfArrival > 0) {
          time = sample_phase +
                 (caloroc_hit.getTimeStamp() + sample_index) * m_cfg.caloroc.time_window -
                 sample.timeOfArrival / static_cast<double>(m_cfg.caloroc.capTOA) *
                     m_cfg.caloroc.dyRangeTOA;
          found_time = true;
        }
      }
    }

    // Encode calibrated energy and time for the established calorimeter reconstruction.
    auto rawhit = rawhits->create();
    rawhit.setCellID(caloroc_hit.getCellID());
    rawhit.setAmplitude(std::clamp(
        std::llround(m_cfg.pedMeanADC + energy / m_cfg.dyRangeADC * m_cfg.capADC), 0LL,
        static_cast<long long>(m_cfg.capADC)));
    rawhit.setTimeStamp(std::max(std::llround(time * m_stepTDC), 0LL));

    // Restore truth relations from the pulse that produced this CALOROC hit.
    const auto pulse = (*pulses)[hit_index];
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
