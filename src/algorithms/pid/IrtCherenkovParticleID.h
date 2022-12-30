// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

// data model
#include <edm4eic/RawPMTHitCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <edm4hep/ParticleIDCollection.h>

// IRT
#include <IRT/CherenkovRadiator.h>
#include <IRT/CherenkovEvent.h>
#include <IRT/CherenkovDetectorCollection.h>

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
      // - `in_raw_hits` is a collection of digitized (raw) sensor hits
      // - `in_charged_particles` is a map of a radiator name to a collection of TrackSegments
      //   - each TrackSegment has a list of TrackPoints: the propagation of reconstructed track (trajectory) points
      // - the output is a collection of particle ID objects
      std::vector<edm4eic::CherenkovParticleID*> AlgorithmProcess(
          std::vector<const edm4eic::RawPMTHit*>& in_raw_hits,
          std::map<std::string,std::vector<const edm4eic::TrackSegment*>>& in_charged_particles
          );

    private:

      std::shared_ptr<spdlog::logger> m_log;
      CherenkovDetectorCollection*    m_irt_det_coll;
      CherenkovDetector*              m_irt_det;
      unsigned                        m_algorithm_id;

      bool        m_init_failed;
      uint64_t    m_cell_mask;
      std::string m_det_name;
      std::unordered_map<int,double>           m_pdg_mass;
      std::map<std::string,CherenkovRadiator*> m_pid_radiators;

  };
}
