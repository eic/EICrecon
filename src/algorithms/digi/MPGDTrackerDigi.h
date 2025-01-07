// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov

// Derived from "../SiliconTrackerDigi.h".

#pragma once

#include <DD4hep/Detector.h>
#include <DD4hep/Segmentations.h>
#include <DDRec/CellIDPositionConverter.h>
#include <TRandomGen.h>
#include <algorithms/algorithm.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <functional>
#include <string>
#include <string_view>

#include "MPGDTrackerDigiConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using MPGDTrackerDigiAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::SimTrackerHitCollection>,
                          algorithms::Output<edm4eic::RawTrackerHitCollection,
                                             edm4eic::MCRecoTrackerHitAssociationCollection>>;

class MPGDTrackerDigi : public MPGDTrackerDigiAlgorithm,
                           public WithPodConfig<MPGDTrackerDigiConfig> {

public:
  MPGDTrackerDigi(std::string_view name)
      : MPGDTrackerDigiAlgorithm{name,
                                    {"inputHitCollection"},
                                    {"outputRawHitCollection", "outputHitAssociations"},
                                    "2D-strip segmentation, apply threshold, digitize within ADC range, "
                                    "convert time with smearing resolution."} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  /** Random number generation*/
  TRandomMixMax m_random;
  std::function<double()> m_gauss;
  // FIXME replace with standard random engine
  // std::default_random_engine generator; // TODO: need something more appropriate here
  // std::normal_distribution<double> m_normDist; // defaults to mean=0, sigma=1

  // algorithms::Generator m_rng = algorithms::RandomSvc::instance().generator();

  /** Segmentation */
  const dd4hep::Detector* m_detector{nullptr};
  const dd4hep::rec::CellIDPositionConverter* m_cellid_converter{nullptr};
  dd4hep::Segmentation m_seg;
};

} // namespace eicrecon
