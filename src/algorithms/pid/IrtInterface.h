//
// Copyright 2025, Alexander Kiselev
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifdef WITH_IRT2_SUPPORT

#pragma once

class TFile;
class TTree;
class TBranch;

#include <IRT2/CherenkovDetector.h>
#include <IRT2/CherenkovEvent.h>
#include <IRT2/ReconstructionFactory.h>
#include <TRandomGen.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/IrtParticleCollection.h>
#include <edm4eic/IrtRadiatorInfoCollection.h>
#include <edm4eic/MCRecoTrackParticleAssociationCollection.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <spdlog/logger.h>
#include <functional>
#include <memory>
#include <string>
#include <string_view>

#include "IrtInterfaceConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {
using IrtInterfaceAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::MCParticleCollection, edm4eic::TrackCollection,
                      edm4eic::MCRecoTrackParticleAssociationCollection,
                      edm4eic::TrackSegmentCollection, edm4hep::SimTrackerHitCollection>,
    algorithms::Output<edm4eic::IrtRadiatorInfoCollection, edm4eic::IrtParticleCollection>>;

class IrtInterface : public IrtInterfaceAlgorithm, public WithPodConfig<IrtConfig> {

public:
  IrtInterface(std::string_view name)
      : IrtInterfaceAlgorithm{name,
                              {"inputMCParticles", "inputTracks", "inputTrackAssotiations",
                               "inputTrackSegments", "inputSimHits"},
                              {"outputIrtRadiatorInfo", "outputIrtParticles"},
                              "Performs PID evaluation based on IRT2 algorithm"}
      , m_Event(0)
      , m_EventPtr(0)
      //, m_Instance(0)
      //, m_OutputFileCreated(false)
      , m_ProcessedEventsPtr(new unsigned int())
      , m_OutputFile(0)
      , m_EventTree(0)
      , m_EventBranch(0)
      , m_ReconstructionFactory(0)
      , m_EventTreeOutputEnabled(true)
      , m_CombinedPlotVisualizationEnabled(false)
      , m_wtopx(0)
      , m_wtopy(0)
      , m_wx(0)
      , m_wy(0) {};

  void init() final;

  void JsonParser(void);

  void process(const Input&, const Output&) const final;

  ~IrtInterface();

private:
  std::shared_ptr<spdlog::logger> m_log;
  //IRT2::CherenkovDetector* m_irt_det;

  // m_EventPtr: need this because process() is const;
  IRT2::CherenkovEvent *m_Event, **m_EventPtr;

  // Pointer: process() is const;
  unsigned* m_ProcessedEventsPtr;

  //unsigned m_Instance;
  //bool m_OutputFileCreated;

  std::string m_OutputFileName;

  TFile* m_OutputFile;
  TTree* m_EventTree;
  //static std::map<std::string, unsigned> m_InstanceCounters;
  TBranch* m_EventBranch;

  TRandomMixMax m_random;
  std::function<double()> m_rngUni;

  IRT2::ReconstructionFactory* m_ReconstructionFactory;
  bool m_EventTreeOutputEnabled, m_CombinedPlotVisualizationEnabled;
  int m_wtopx;
  unsigned m_wtopy, m_wx, m_wy;

  const algorithms::GeoSvc& m_geo = algorithms::GeoSvc::instance();
};
} // namespace eicrecon

#endif
