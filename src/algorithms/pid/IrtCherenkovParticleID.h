// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <IRT2/CherenkovDetector.h>
#include <IRT2/CherenkovDetectorCollection.h>
#include <IRT2/CherenkovRadiator.h>
#include <algorithms/algorithm.h>
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <stdint.h>
#include <map>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

// EICrecon
#include "algorithms/interfaces/WithPodConfig.h"
#include "services/particle/ParticleSvc.h"
#include "algorithms/pid/IrtCherenkovParticleIDConfig.h"

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
                               public WithPodConfig<IrtCherenkovParticleIDConfig> {

public:
  IrtCherenkovParticleID(std::string_view name)
      : IrtCherenkovParticleIDAlgorithm{name,
                                        {"inputAerogelTrackSegments", "inputGasTrackSegments",
                                         "inputMergedTrackSegments", "inputRawHits",
                                         "inputRawHitAssociations"},
                                        {"outputAerogelParticleIDs", "outputGasParticleIDs"},
                                        "Effectively 'zip' the input particle IDs"} {}

  // FIXME: init() must not take arguments
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
  void init(IRT2::CherenkovDetectorCollection* irt_det_coll);
#pragma GCC diagnostic pop

  void process(const Input&, const Output&) const;

private:
  // any access (R or W) to m_irt_det_coll, m_irt_det, m_pid_radiators must be locked
  inline static std::mutex m_irt_det_mutex;
  IRT2::CherenkovDetectorCollection* m_irt_det_coll;
  IRT2::CherenkovDetector* m_irt_det;
  std::map<std::string, IRT2::CherenkovRadiator*> m_pid_radiators;

  const algorithms::ParticleSvc& m_particleSvc = algorithms::ParticleSvc::instance();

  uint64_t m_cell_mask;
  std::string m_det_name;
  std::unordered_map<int, double> m_pdg_mass;
};

} // namespace eicrecon
