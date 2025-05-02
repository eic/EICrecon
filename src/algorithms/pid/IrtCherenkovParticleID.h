// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <IRT/CherenkovDetector.h>
#include <IRT/CherenkovDetectorCollection.h>
#include <IRT/CherenkovRadiator.h>
#include <algorithms/algorithm.h>
#include <algorithms/logger.h>
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <fmt/core.h>
#include <stdint.h>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

// EICrecon
#include "algorithms/interfaces/ParticleSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/pid/IrtCherenkovParticleIDConfig.h"
#include "algorithms/pid/Tools.h"

namespace eicrecon {

// - `in_raw_hits` is a collection of digitized (raw) sensor hits, possibly including noise hits
// - `in_hit_assocs` is a collection of digitized (raw) sensor hits, associated with MC (simulated) hits;
//   noise hits are not included since there is no associated simulated photon
// - `in_charged_particles` is a map of a radiator name to a collection of TrackSegments
//   - each TrackSegment has a list of TrackPoints: the propagation of reconstructed track (trajectory) points
// - the output is a map: radiator name -> collection of particle ID objects
using IrtCherenkovParticleIDAlgorithm = algorithms::Algorithm<
    algorithms::Input<const edm4eic::TrackSegmentCollection, const edm4eic::TrackSegmentCollection,
                      const edm4eic::TrackSegmentCollection, const edm4eic::RawTrackerHitCollection,
                      const edm4eic::MCRecoTrackerHitAssociationCollection>,
    algorithms::Output<edm4eic::CherenkovParticleIDCollection,
                       edm4eic::CherenkovParticleIDCollection>>;

class IrtCherenkovParticleID : public IrtCherenkovParticleIDAlgorithm,
                               public WithPodConfig<IrtCherenkovParticleIDConfig>,
                               private Tools<IrtCherenkovParticleID> {

public:
  IrtCherenkovParticleID(std::string_view name)
      : IrtCherenkovParticleIDAlgorithm{name,
                                        {"inputAerogelTrackSegments", "inputGasTrackSegments",
                                         "inputMergedTrackSegments", "inputRawHits",
                                         "inputRawHitAssociations"},
                                        {"outputAerogelParticleIDs", "outputGasParticleIDs"},
                                        "Effectively 'zip' the input particle IDs"}
      , Tools(this) {}

  // FIXME: init() must not take arguments
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
  void init(CherenkovDetectorCollection* irt_det_coll);
#pragma clang diagnostic pop
#pragma GCC diagnostic pop

  void process(const Input&, const Output&) const;

private:
  template <algorithms::LogLevel lvl, typename... T>
  constexpr void log(fmt::format_string<T...> fmt, T&&... args) const {
#if algorithms_VERSION_MAJOR > 1 || (algorithms_VERSION_MAJOR == 1 && algorithms_VERSION_MINOR > 1)
    algorithms::LoggerMixin::report_fmt<lvl>(fmt, std::forward<decltype(args)>(args)...);
#else
    // all logging to info
    info(fmt::format(fmt, std::forward<decltype(args)>(args)...));
#endif
  }

  friend class Tools;

private:
  CherenkovDetectorCollection* m_irt_det_coll;
  CherenkovDetector* m_irt_det;

  const algorithms::ParticleSvc& m_particleSvc = algorithms::ParticleSvc::instance();

  uint64_t m_cell_mask;
  std::string m_det_name;
  std::unordered_map<int, double> m_pdg_mass;
  std::map<std::string, CherenkovRadiator*> m_pid_radiators;
};

} // namespace eicrecon
