// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <IRT/CherenkovDetector.h>
#include <IRT/CherenkovDetectorCollection.h>
#include <IRT/CherenkovRadiator.h>
#include <algorithms/algorithm.h>
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <spdlog/logger.h>
#include <stdint.h>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

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

  void init(CherenkovDetectorCollection* irt_det_coll);

  void process(const Input&, const Output&) const;

private:
  template <algorithms::LogLevel lvl, typename... T>
  constexpr void log(fmt::format_string<T...> fmt, T&&... args) const {
    log<lvl>(fmt, std::forward<decltype(args)>(args)...);
  }

  friend class IrtCherenkovParticleIDConfig;
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

// Definition of IrtCherenkovParticleIDConfig::Print* requires class IrtCherenkovParticleID, but
// circular dependency prevents it from being in IrtCherenkovParticleIDConfig.h

// print warnings about cheat modes
template <algorithms::LogLevel lvl>
void IrtCherenkovParticleIDConfig::PrintCheats(const eicrecon::IrtCherenkovParticleID* logger,
                                               bool printAll) {
  auto print_param = [&logger, &printAll](auto name, bool val, auto desc) {
    if (printAll)
      logger->log<lvl>("  {:>20} = {:<}", name, val);
    else if (val)
      logger->warning("CHEAT MODE '{}' ENABLED: {}", name, desc);
  };
  print_param("cheatPhotonVertex", cheatPhotonVertex,
              "use MC photon vertex, wavelength, refractive index");
  print_param("cheatTrueRadiator", cheatTrueRadiator, "use MC truth to obtain true radiator");
}

// print all parameters
template <algorithms::LogLevel lvl>
void IrtCherenkovParticleIDConfig::Print(const eicrecon::IrtCherenkovParticleID* logger) {
  logger->log<lvl>("{:=^60}", " IrtCherenkovParticleIDConfig Settings ");
  auto print_param = [&logger](auto name, auto val) {
    logger->log<lvl>("  {:>20} = {:<}", name, val);
  };
  print_param("numRIndexBins", numRIndexBins);
  PrintCheats<lvl>(logger, true);
  logger->log<lvl>("pdgList:");
  for (const auto& pdg : pdgList)
    logger->log<lvl>("  {}", pdg);
  for (const auto& [name, rad] : radiators) {
    logger->log<lvl>("{:-<60}", fmt::format("--- {} config ", name));
    print_param("smearingMode", rad.smearingMode);
    print_param("smearing", rad.smearing);
    print_param("referenceRIndex", rad.referenceRIndex);
    print_param("attenuation", rad.attenuation);
  }
  logger->log<lvl>("{:=^60}", "");
}

} // namespace eicrecon
