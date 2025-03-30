//
// Copyright 2025, Alexander Kiselev
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <TRandomGen.h>
//#include <TRandom.h>

#include <IRT/CherenkovRadiator.h>

#include <algorithms/algorithm.h>
#include <edm4hep/MCParticleCollection.h>
#include "edm4eic/IrtDebugInfoCollection.h"
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/MCRecoTrackParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>

#include <spdlog/logger.h>
#include "services/geometry/richgeo/RichGeo_service.h"

class TTree;
class TFile;
class TH1D;
class TBranch;
class CherenkovEvent;
//#include <mutex>

namespace eicrecon {
  using IrtDebuggingAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      const edm4hep::MCParticleCollection,
      edm4eic::ReconstructedParticleCollection,
      edm4eic::MCRecoParticleAssociationCollection,
      const edm4eic::TrackSegmentCollection,
      const edm4hep::SimTrackerHitCollection
      >,
    algorithms::Output<
      edm4eic::IrtDebugInfoCollection
      >
    >;

  class IrtDebugging
    : public IrtDebuggingAlgorithm/*,
				    public WithPodConfig<IrtCherenkovParticleIDConfig>*/ {

  public:
    IrtDebugging(std::string_view name)
      : m_Event(0), m_EventPtr(0), m_Instance(0), IrtDebuggingAlgorithm{name,
                            {
			      "inputMCParticles",
			      "inputRecoParticles",
			      "inputMCRecoAssotiations",
			      "inputAerogelTrackSegments",
			      "inputSimHits"
			    },
			    // This part is not activated as of now;
			    {"outputAerogelParticleIDs"},
			    "Effectively 'zip' the input particle IDs"} {};
    
    void init(RichGeo_service &service, std::shared_ptr<spdlog::logger>& logger);

    void process(const Input&, const Output&) const;
    
    ~IrtDebugging();

    //std::mutex m_OutputTreeMutex;
  
  private:
    std::shared_ptr<spdlog::logger> m_log;
    // FIXME: make static?;
    CherenkovDetectorCollection*    m_irt_det_coll;
    CherenkovDetector*              m_irt_det;

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
    
    std::string m_det_name;
  };
}
