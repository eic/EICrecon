// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#if 1//_TODAY_
#include <IRT/CherenkovDetector.h>
#include <IRT/CherenkovDetectorCollection.h>
#endif
#include <IRT/CherenkovRadiator.h>

#include <algorithms/algorithm.h>
//#include <qq.h>
//#include <edm4eic/CherenkovParticleIDCollection.h>
#include "edm4eic/IrtDebugInfoCollection.h"
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/TrackSegmentCollection.h>

#if _TODAY_
#include <spdlog/logger.h>
#include <stdint.h>
#include <map>
#include <memory>
#include <string>
#endif
#include <string_view>
#if _TODAY_
#include <unordered_map>

// EICrecon
#include "IrtCherenkovParticleIDConfig.h"
#include "algorithms/interfaces/ParticleSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"
#endif

namespace eicrecon {
#if 1//_TODAY_
  // - `in_raw_hits` is a collection of digitized (raw) sensor hits, possibly including noise hits
  // - `in_hit_assocs` is a collection of digitized (raw) sensor hits, associated with MC (simulated) hits;
  //   noise hits are not included since there is no associated simulated photon
  // - `in_charged_particles` is a map of a radiator name to a collection of TrackSegments
  //   - each TrackSegment has a list of TrackPoints: the propagation of reconstructed track (trajectory) points
  // - the output is a map: radiator name -> collection of particle ID objects
  //using IrtCherenkovParticleIDAlgorithm = algorithms::Algorithm<
  using IrtDebuggingAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      const edm4eic::TrackSegmentCollection,
      const edm4hep::SimTrackerHitCollection//,
      //const edm4eic::TrackSegmentCollection,
      //const edm4eic::TrackSegmentCollection,
      //const edm4eic::RawTrackerHitCollection,
      //const edm4eic::MCRecoTrackerHitAssociationCollection
    >,
    algorithms::Output<
      //edm4eic::CherenkovParticleIDCollection,
      //edm4eic::CherenkovParticleIDCollection
      edm4eic::IrtDebugInfoCollection
      >
  >;
#endif

  class IrtDebugging
  : public IrtDebuggingAlgorithm/*,
    public WithPodConfig<IrtCherenkovParticleIDConfig>*/ {

  public:
    IrtDebugging(std::string_view name)// {}
#if 1//_TODAY_
      : IrtDebuggingAlgorithm{name,
                            {
			      "inputAerogelTrackSegments", "inputSimHits"//, //"inputGasTrackSegments", "inputMergedTrackSegments",
                              //"inputRawHits", "inputRawHitAssociations"
                            },
                            {"outputAerogelParticleIDs"/*, "outputGasParticleIDs"*/},
                            "Effectively 'zip' the input particle IDs"} {}
#endif
    
    void init(CherenkovDetectorCollection* irt_det_coll/*, std::shared_ptr<spdlog::logger>& logger*/);

    void process(const Input&, const Output&) const;

  private:
    //std::shared_ptr<spdlog::logger> m_log;
    CherenkovDetectorCollection*    m_irt_det_coll;
    CherenkovDetector*              m_irt_det;

#if _TODAY_
    const algorithms::ParticleSvc& m_particleSvc = algorithms::ParticleSvc::instance();

    uint64_t    m_cell_mask;
#endif
    std::string m_det_name;
#if _TODAY_
    std::unordered_map<int,double>           m_pdg_mass;
#endif
    std::map<std::string,CherenkovRadiator*> m_pid_radiators;
  };
}
