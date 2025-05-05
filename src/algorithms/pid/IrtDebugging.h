//
// Copyright 2025, Alexander Kiselev
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <nlohmann/json.hpp>

#include <TRandomGen.h>
//#include <TRandom.h>

#include <algorithms/algorithm.h>
#include <edm4hep/MCParticleCollection.h>
#include "edm4eic/IrtDebugInfoCollection.h"
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
//#include <edm4eic/MCRecoTrackParticleAssociationCollection.h>
//#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>

#include <spdlog/logger.h>
#include "services/geometry/dd4hep/DD4hep_service.h"

#include <IRT/CherenkovRadiator.h>

class TTree;
class TFile;
class TH1D;
class TBranch;
class CherenkovEvent;
class CherenkovDetectorCollection;
class CherenkovDetector;

//#include <mutex>

// JOmniFactoryGeneratorT does not allow to use omre than one extra config parameter ->
// bunch whatever is needed to pass in a single structure; do not want to repeat parsing
// of either th eoptics file or a JSON configuration file twice;
struct IrtDebuggingConfig {
  IrtDebuggingConfig(): m_irt_geometry(0), m_eta_min(0.0), m_eta_max(0.0) {};
  
  CherenkovDetectorCollection *m_irt_geometry;
  nlohmann::json m_json_config;
  
  // FIXME: do it better later; but in general see no reason to parse the same fields
  // in a JSON file twice;
  double m_eta_min, m_eta_max;
};

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
    : public IrtDebuggingAlgorithm {

  public:
    IrtDebugging(std::string_view name)
      : m_Event(0), m_EventPtr(0), m_Instance(0), m_sign(0.0), IrtDebuggingAlgorithm{
	  name,
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
    
    void init(DD4hep_service &service, IrtDebuggingConfig &config, std::shared_ptr<spdlog::logger>& logger);

    void process(const Input&, const Output&) const;
    
    ~IrtDebugging();

    //std::mutex m_OutputTreeMutex;
  
  private:
    std::shared_ptr<spdlog::logger> m_log;
    // FIXME: make static?;
    //CherenkovDetectorCollection*    m_irt_det_coll;
    CherenkovDetector*              m_irt_det;

    // m_EventPtr: need this because process() is const;
    /*static thread_local*/ CherenkovEvent *m_Event, **m_EventPtr;
#if _LATER_
    static TFile *m_OutputFile;
    static TTree *m_EventTree;
    static unsigned m_InstanceCounter;
    static TBranch *m_EventBranch;
#endif
    unsigned m_Instance;

    std::string m_OutputFileName;
    //+static TH1D *m_Debug;
    
    TRandomMixMax m_random;
    std::function<double()> m_rngUni;
    
    //std::string m_det_name;

    IrtDebuggingConfig m_config;

    // FIXME: do it better later;
    double m_sign;//, m_eta_min, m_eta_max;
  };
}
