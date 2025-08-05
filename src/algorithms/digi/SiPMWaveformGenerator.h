// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson

#ifndef EICRECON_SIPMWAVEFORMGENERATOR_H
#define EICRECON_SIPMWAVEFORMGENERATOR_H

#include <algorithm>
#include <algorithms/algorithm.h>
#include <edm4hep/RawTimeSeriesCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <string_view>

#include "SiPMWaveformGeneratorConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

// --------------------------------------------------------------------------
//! Algorithm input/output
// --------------------------------------------------------------------------
using SiPMWaveformGeneratorAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::SimCalorimeterHitCollection>,
                          algorithms::Output<edm4hep::RawTimeSeriesCollection>>;

// --------------------------------------------------------------------------
//! SiPM Waveform Generator
// --------------------------------------------------------------------------
/*! An algorithm which takes a collection of simulated calorimeter hits
   *  (edm4hep::SimCalorimeterHit) and generates a waveform (stored as an
   *  edm4hep::RawTimeSeries) for each.
   */
class SiPMWaveformGenerator : public SiPMWaveformGeneratorAlgorithm,
                              public WithPodConfig<SiPMWaveformGeneratorConfig> {

public:
  // ctor
  SiPMWaveformGenerator(std::string_view name)
      : SiPMWaveformGeneratorAlgorithm{
            name,
            {"inputSimCaloHitCollection"},
            {"outputRawTimeSeriesCollection"},
            "Generates SiPM-esque waveforms for each simulated calorimeter hit."} {}

  // public methods
  void init();
  void process(const Input&, const Output&) const final;

}; // end SiPMWaveformGenerator

} // namespace eicrecon

#endif
