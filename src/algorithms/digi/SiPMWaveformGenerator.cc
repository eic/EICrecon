// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson

#include <array> // FIXME can be removed once SimSiPM is implemented
#include <edm4hep/RawTimeSeries.h>
#include <edm4hep/SimCalorimeterHit.h>

#include "SiPMWaveformGenerator.h"
#include "algorithms/digi/SiPMWaveformGeneratorConfig.h"

namespace eicrecon {

// --------------------------------------------------------------------------
//! Initialize algorithm
// --------------------------------------------------------------------------
void SiPMWaveformGenerator::init() { /* TODO fill in */ } // end 'init()'

// --------------------------------------------------------------------------
//! Process inputs
// --------------------------------------------------------------------------
void SiPMWaveformGenerator::process(const SiPMWaveformGenerator::Input& input,
                                    const SiPMWaveformGenerator::Output& output) const {

  // grab inputs/outputs
  const auto [in_simhits] = input;
  auto [out_waveforms]    = output;

  // TEST WAVEFORM
  // - FIXME this will be replaced by
  //   running SimSiPM!
  std::array<double, 112> arrTestAdcCounts = {
      -4.42797e-06, 0.215669, 0.853029, 2.06377, 10.6817, 42.5832, 142.85,  338.127, 542.434,
      729.222,      874.949,  963.295,  1000,    993.012, 956.098, 902.16,  840.479, 773.752,
      709.145,      647.323,  592.973,  544.065, 500.331, 460.647, 428.445, 399.501, 373.974,
      350.113,      327.552,  306.732,  286.298, 267.809, 251.647, 235.589, 221.29,  206.117,
      193.011,      180.853,  168.628,  156.599, 145.884, 136.61,  128.631, 120.27,  112.169,
      105.076,      97.4752,  90.6311,  85.0997, 79.6648, 75.1151, 69.7922, 65.4247, 61.8002,
      57.7207,      53.7169,  49.8135,  46.6682, 44.2817, 41.5321, 39.1095, 36.5253, 33.7032,
      31.4861,      29.3692,  27.5674,  26.5272, 24.4347, 22.9634, 21.2692, 19.5383, 18.5056,
      17.5114,      16.8001,  16.3486,  15.296,  14.3898, 13.4473, 12.2312, 11.4882, 10.8308,
      10.262,       10.2914,  9.52024,  9.1836,  8.58881, 8.11792, 7.87216, 7.1251,  6.87886,
      7.21701,      6.46223,  6.20167,  5.93047, 5.39531, 5.09526, 4.97851, 4.79073, 5.1291,
      4.73879,      4.76006,  4.48906,  4.33813, 4.32425, 3.98607, 4.05054, 4.3771,  4.02732,
      3.91601,      3.8918,   3.56212,  3.46904};

  // generate waveform for each input hit
  //  - n.b. merging/summing waveforms
  //    handled downstream
  for (const auto& simhit : *in_simhits) {

    // create waveform in output collection
    edm4hep::MutableRawTimeSeries waveform = out_waveforms->create();
    waveform.setCellID(simhit.getCellID());
    waveform.setTime(1.);                              // FIXME this is a placeholder value!
    waveform.setCharge(1.);                            // FIXME this is a placeholder value!
    waveform.setInterval(1. / (double)m_cfg.nSamples); // FIXME this is a placeholder value!

    // set samples
    for (size_t iSample = 0; iSample < m_cfg.nSamples; ++iSample) {
      waveform.addToAdcCounts((int32_t)arrTestAdcCounts.at(iSample));
    }
  } // end sim hit loop

} // end 'process(Input&, Output&)'

} // namespace eicrecon
