// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <string>
#include <string_view>

#include "SiliconTrackerDigiConfig.h"
#include "algorithms/interfaces/UniqueIDGenSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"

#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
#include <edm4eic/MCRecoTrackerHitLinkCollection.h>
#endif

namespace eicrecon {

using SiliconTrackerDigiAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::EventHeaderCollection, edm4hep::SimTrackerHitCollection>,
    algorithms::Output<edm4eic::RawTrackerHitCollection,
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                       edm4eic::MCRecoTrackerHitLinkCollection,
#endif
                       edm4eic::MCRecoTrackerHitAssociationCollection>>;

class SiliconTrackerDigi : public SiliconTrackerDigiAlgorithm,
                           public WithPodConfig<SiliconTrackerDigiConfig> {

public:
  SiliconTrackerDigi(std::string_view name)
      : SiliconTrackerDigiAlgorithm{name,
                                    {"eventHeaderCollection", "inputHitCollection"},
                                    {"outputRawHitCollection",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                     "outputHitLinks",
#endif
                                     "outputHitAssociations"},
                                    "Apply threshold, digitize within ADC range, "
                                    "convert time with smearing resolution."} {
  }

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const algorithms::UniqueIDGenSvc& m_uid = algorithms::UniqueIDGenSvc::instance();
};

} // namespace eicrecon
