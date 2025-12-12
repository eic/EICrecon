//
// Copyright 2025, Alexander Kiselev
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <nlohmann/json.hpp>

#include <TRandomGen.h>

#include <algorithms/algorithm.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4eic/IrtRadiatorInfoCollection.h>
#include <edm4eic/IrtParticleCollection.h>
#include <edm4eic/IrtEventCollection.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>

#include <spdlog/logger.h>
#include "services/geometry/dd4hep/DD4hep_service.h"

#include <IRT2/CherenkovRadiator.h>
using namespace IRT2;

class TTree;
class TFile;
class TH1D;
class TBranch;
namespace IRT2 {
class ReconstructionFactory;
class CherenkovEvent;
class CherenkovDetectorCollection;
class CherenkovDetector;
} // namespace IRT2
// JOmniFactoryGeneratorT does not allow to use more than one extra config parameter ->
// bunch whatever is needed to pass in a single structure; do not want to repeat parsing
// of either the optics file or a JSON configuration file twice;
struct IrtConfig {
  IrtConfig() : m_irt_geometry(0), m_eta_min(0.0), m_eta_max(0.0) {};

  //IRT2::CherenkovDetectorCollection *m_irt_geometry;
  CherenkovDetectorCollection* m_irt_geometry;
  nlohmann::json m_json_config;

  // FIXME: perhaps do it better later; but in general see no reason in parsing
  // the same fields in a JSON file twice;
  double m_eta_min, m_eta_max;
};

namespace eicrecon {
using IrtInterfaceAlgorithm = algorithms::Algorithm<
    algorithms::Input<const edm4hep::MCParticleCollection, edm4eic::ReconstructedParticleCollection,
                      edm4eic::MCRecoParticleAssociationCollection,
                      const edm4eic::TrackSegmentCollection,
                      const edm4hep::SimTrackerHitCollection>,
    algorithms::Output<edm4eic::IrtRadiatorInfoCollection, edm4eic::IrtParticleCollection,
                       edm4eic::IrtEventCollection>>;

class IrtInterface : public IrtInterfaceAlgorithm {

public:
  IrtInterface(std::string_view name)
      : m_Event(0)
      , m_EventPtr(0)
      , m_Instance(0)
      , m_ReconstructionFactory(0)
      , IrtInterfaceAlgorithm{name,
                              {"inputMCParticles", "inputRecoParticles", "inputMCRecoAssotiations",
                               "inputTrackSegments", "inputSimHits"},
                              // This part is not activated as of now;
                              {"outputAerogelParticleIDs"},
                              "Effectively 'zip' the input particle IDs"} {};

  void init(DD4hep_service& service, IrtConfig& config, std::shared_ptr<spdlog::logger>& logger);

  void JsonParser(void);

  void process(const Input&, const Output&) const;

  ~IrtInterface();

private:
  std::shared_ptr<spdlog::logger> m_log;
  //using IRT2::CherenkovDetector;
  /*IRT2::*/ CherenkovDetector* m_irt_det;

  // m_EventPtr: need this because process() is const;
  /*static thread_local*/
  //using IRT2::CherenkovEvent;
  /*IRT2::*/ CherenkovEvent *m_Event, **m_EventPtr;

  unsigned m_Instance;

  std::string m_OutputFileName;

  TRandomMixMax m_random;
  std::function<double()> m_rngUni;

  IrtConfig m_config;

  //using IRT2::ReconstructionFactory;
  /*IRT2::*/ ReconstructionFactory* m_ReconstructionFactory;
};
} // namespace eicrecon
