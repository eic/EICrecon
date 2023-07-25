// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

// data model
#include <edm4hep/MCParticle.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/CherenkovParticleIDCollection.h>

// IRT
#include <IRT/CherenkovRadiator.h>
#include <IRT/CherenkovEvent.h>
#include <IRT/CherenkovDetectorCollection.h>
#include <IRT/OpticalPhoton.h>

// DD4hep
#include <Evaluator/DD4hepUnits.h>

// EICrecon
#include "IrtCherenkovParticleIDConfig.h"
#include "Tools.h"
#include <algorithms/interfaces/WithPodConfig.h>
#include <spdlog/spdlog.h>

namespace eicrecon {

  class IrtCherenkovParticleID : public WithPodConfig<IrtCherenkovParticleIDConfig> {

    public:
      IrtCherenkovParticleID() = default;
      ~IrtCherenkovParticleID() {}

      void AlgorithmInit(
          CherenkovDetectorCollection*     irt_det_coll,
          std::shared_ptr<spdlog::logger>& logger
          );
      void AlgorithmChangeRun();

      // AlgorithmProcess
      // - `in_raw_hits` is a collection of digitized (raw) sensor hits, possibly including noise hits
      // - `in_hit_assocs` is a collection of digitized (raw) sensor hits, associated with MC (simulated) hits;
      //   noise hits are not included since there is no associated simulated photon
      // - `in_charged_particles` is a map of a radiator name to a collection of TrackSegments
      //   - each TrackSegment has a list of TrackPoints: the propagation of reconstructed track (trajectory) points
      // - the output is a map: radiator name -> collection of particle ID objects
      std::map<std::string, std::unique_ptr<edm4eic::CherenkovParticleIDCollection>> AlgorithmProcess(
          std::map<std::string, const edm4eic::TrackSegmentCollection*>& in_charged_particles,
          const edm4eic::RawTrackerHitCollection*                        in_raw_hits,
          const edm4eic::MCRecoTrackerHitAssociationCollection*          in_hit_assocs
          );

    private:

      std::shared_ptr<spdlog::logger> m_log;
      CherenkovDetectorCollection*    m_irt_det_coll;
      CherenkovDetector*              m_irt_det;

      uint64_t    m_cell_mask;
      std::string m_det_name;
      std::unordered_map<int,double>           m_pdg_mass;
      std::map<std::string,CherenkovRadiator*> m_pid_radiators;

  };
}
