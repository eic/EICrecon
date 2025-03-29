// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <TRandomGen.h>
//#include <TRandom.h>

#if 1//_TODAY_
#include <IRT/CherenkovDetector.h>
#include <IRT/CherenkovDetectorCollection.h>
#endif
#include <IRT/CherenkovRadiator.h>

#include <algorithms/algorithm.h>
//#include <qq.h>
//#include <edm4eic/CherenkovParticleIDCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include "edm4eic/IrtDebugInfoCollection.h"
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
//#include <edm4eic/MCRecoParticleAssociation.h>
#include <edm4eic/MCRecoTrackParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>

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

#include "services/geometry/richgeo/RichGeo_service.h"

class TTree;
class TFile;
class TH1D;
class TBranch;
class CherenkovEvent;
//#include <mutex>

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
      const edm4hep::MCParticleCollection,
  //const edm4eic::TrackCollection,
      //const edm4eic::MCRecoParticleAssociationCollection,
  //const edm4eic::MCRecoTrackParticleAssociationCollection,
    edm4eic::ReconstructedParticleCollection,//> m_recoparticles_output{this};
    edm4eic::MCRecoParticleAssociationCollection,//> m_recoassocs_output{this};
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
      : /*m_ThreadCounter(0),*/ m_Event(0), m_EventPtr(0), m_Instance(0), IrtDebuggingAlgorithm{name,
                            {
  "inputMCParticles",
    "inputRecoParticles",
    "inputMCRecoAssotiations",
    "inputAerogelTrackSegments", "inputSimHits"//, //"inputGasTrackSegments", "inputMergedTrackSegments",
    //"inputRawHits", "inputRawHitAssociations"
    },
      {"outputAerogelParticleIDs"/*, "outputGasParticleIDs"*/},
    "Effectively 'zip' the input particle IDs"} {}
#endif
    
      //+void init(CherenkovDetectorCollection* irt_det_coll/*, std::shared_ptr<spdlog::logger>& logger*/);
      void init(RichGeo_service &service);

    void process(const Input&, const Output&) const;
    ~IrtDebugging();// {};

    //std::mutex m_OutputTreeMutex;
  
  private:
    //std::shared_ptr<spdlog::logger> m_log;
    CherenkovDetectorCollection*    m_irt_det_coll;
    CherenkovDetector*              m_irt_det;

    //private:
    static TFile *m_OutputFile;
    // m_EventPtr: need this because process() is const;
    /*static thread_local*/ CherenkovEvent *m_Event, **m_EventPtr;
    static TTree *m_EventTree;
    static unsigned m_InstanceCounter;
    static TBranch *m_EventBranch;
    unsigned m_Instance;

  static TH1D *m_Debug;
    
    TRandomMixMax m_random;
    std::function<double()> m_rngUni;
    
#if _TODAY_
    const algorithms::ParticleSvc& m_particleSvc = algorithms::ParticleSvc::instance();
#endif
    //uint64_t    m_cell_mask;
    std::string m_det_name;
#if _TODAY_
    std::unordered_map<int,double>           m_pdg_mass;
#endif
    std::map<std::string,CherenkovRadiator*> m_pid_radiators;
  };
}
