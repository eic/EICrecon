// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, 2025, 2026, Alexander Kiselev

#ifdef WITH_IRT2_SUPPORT

#pragma once

#include <CherenkovDetector.h>
#include <CherenkovDetectorCollection.h>
#include <IRT2/CherenkovEvent.h>
#include <IRT2/ReconstructionFactory.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/IrtParticleCollection.h>
#include <edm4eic/IrtRadiatorInfoCollection.h>
#include <edm4eic/MCRecoTrackParticleAssociationCollection.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <string>
#include <string_view>

#include "IrtInterfaceConfig.h"
#include "algorithms/interfaces/UniqueIDGenSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {
using IrtInterfaceAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::EventHeaderCollection, edm4hep::MCParticleCollection,
                      edm4eic::TrackCollection, edm4eic::MCRecoTrackParticleAssociationCollection,
                      edm4eic::TrackSegmentCollection, edm4hep::SimTrackerHitCollection>,
    algorithms::Output<edm4eic::IrtRadiatorInfoCollection, edm4eic::IrtParticleCollection>>;

class IrtInterface : public IrtInterfaceAlgorithm, public WithPodConfig<IrtConfig> {

public:
  IrtInterface(std::string_view name)
      : IrtInterfaceAlgorithm{name,
                              {"eventHeaderCollection", "inputMCParticles", "inputTracks",
                               "inputTrackAssotiations", "inputTrackSegments", "inputSimHits"},
                              {"outputIrtRadiatorInfo", "outputIrtParticles"},
                              "Performs PID evaluation based on IRT2 algorithm"} {};

  void init() final;

  void process(const Input&, const Output&) const final;

  ~IrtInterface();

private:
  std::shared_ptr<spdlog::logger> m_log;

  IRT2::CherenkovDetectorCollection* m_irt_geometry;
  IRT2::CherenkovDetector* m_irt_detector;

  std::unique_ptr<IRT2::CherenkovEvent> m_Event;

  const algorithms::UniqueIDGenSvc& m_uid = algorithms::UniqueIDGenSvc::instance();

  std::unique_ptr<IRT2::ReconstructionFactory> m_ReconstructionFactory;

  const algorithms::GeoSvc& m_geo = algorithms::GeoSvc::instance();
};
} // namespace eicrecon

#endif
