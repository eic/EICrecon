// Created by Minjung Kim (minjung.kim@lbl.gov)

#include "AmbiguitySolver.h"
#include "AmbiguitySolverConfig.h"

#include "Acts/AmbiguityResolution/GreedyAmbiguityResolution.hpp"
#if EDM4EIC_VERSION_MAJOR >= 5
#include <edm4eic/Cov6f.h>
#endif
#include "Acts/Utilities/Logger.hpp"
#include "ActsGeometryProvider.h"
#include "DD4hepBField.h"
#include "extensions/spdlog/SpdlogFormatters.h" // IWYU pragma: keep
#include "extensions/spdlog/SpdlogToActs.h"
#include <Acts/EventData/SourceLink.hpp>
#include <Acts/EventData/TrackContainer.hpp>
#include <Acts/EventData/TrackProxy.hpp>
#include <Acts/EventData/TrackStateType.hpp>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <edm4eic/TrackParametersCollection.h>
#include <functional>
#include <list>
#include <optional>
#include <utility>

namespace eicrecon {

Acts::GreedyAmbiguityResolution::Config
transformConfig(const eicrecon::AmbiguitySolverConfig& cfg) {
  Acts::GreedyAmbiguityResolution::Config result;
  result.maximumSharedHits = cfg.maximumSharedHits;
  result.maximumIterations = cfg.maximumIterations;
  result.nMeasurementsMin  = cfg.nMeasurementsMin;
  return result;
}

std::size_t sourceLinkHash(const Acts::SourceLink& a) {
  return static_cast<std::size_t>(a.get<ActsExamples::IndexSourceLink>().index());
}

bool sourceLinkEquality(const Acts::SourceLink& a, const Acts::SourceLink& b) {
  return a.get<ActsExamples::IndexSourceLink>().index() ==
         b.get<ActsExamples::IndexSourceLink>().index();
}

AmbiguitySolver::AmbiguitySolver() {}

void AmbiguitySolver::init(std::shared_ptr<spdlog::logger> log) {
  m_log         = log;
  m_acts_logger = eicrecon::getSpdlogLogger("AmbiguitySolver", m_log);
  m_acts_cfg    = transformConfig(m_cfg);
  m_core        = std::make_unique<Acts::GreedyAmbiguityResolution>(m_acts_cfg, logger().clone());
}

  std::tuple<
        std::unique_ptr<edm4eic::TrajectoryCollection>,
        std::unique_ptr<edm4eic::TrackParametersCollection>,
        std::unique_ptr<edm4eic::TrackCollection>,
        std::vector<ActsExamples::Trajectories*>,
        std::vector<ActsExamples::ConstTrackContainer*>
    >
AmbiguitySolver::process(std::vector<const ActsExamples::ConstTrackContainer*> input_container) {
  // Assuming ActsExamples::ConstTrackContainer is compatible with Acts::ConstVectorTrackContainer
  // Create track container
  std::vector<ActsExamples::ConstTrackContainer*> output_tracks;
  auto track_parameters = std::make_unique<edm4eic::TrackParametersCollection>();

  for (const auto& input_trks : input_container) {

    Acts::GreedyAmbiguityResolution::State state;
    m_core->computeInitialState(*input_trks, state, &sourceLinkHash, &sourceLinkEquality);
    std::cout << "from ACTS: " << state.numberOfTracks << std::endl;
    m_core->resolve(state);

    std::cout << "Resolved to " << state.selectedTracks.size() << " tracks from "
              << input_trks->size() << std::endl;

    ActsExamples::TrackContainer solvedTracks{std::make_shared<Acts::VectorTrackContainer>(),
                                              std::make_shared<Acts::VectorMultiTrajectory>()};
    solvedTracks.ensureDynamicColumns(*input_trks);

    for (auto iTrack : state.selectedTracks) {
      auto destProxy = solvedTracks.getTrack(solvedTracks.addTrack());
      auto srcProxy  = input_trks->getTrack(state.trackTips.at(iTrack));
      destProxy.copyFrom(srcProxy, false);
      destProxy.tipIndex() = srcProxy.tipIndex();
    }
    output_tracks.push_back(new ActsExamples::ConstTrackContainer(
        std::make_shared<Acts::ConstVectorTrackContainer>(std::move(solvedTracks.container())),
        input_trks->trackStateContainerHolder()));
  }

  auto trajectories     = std::make_unique<edm4eic::TrajectoryCollection>();
  auto track_parameters = std::make_unique<edm4eic::TrackParametersCollection>();
  auto tracks           = std::make_unique<edm4eic::TrackCollection>();
  return std::make_tuple(std::move(track_parameters), std::move(output_tracks));
}

} // namespace eicrecon
