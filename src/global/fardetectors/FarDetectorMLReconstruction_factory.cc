// Created by Simon Gardner to do FarDetectorML Tagger reconstruction
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3f.h>
#include <edm4eic/Cov4f.h>
#include <filesystem>

#include "FarDetectorMLReconstruction_factory.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include <ROOT/RVec.hxx>
#include <TDecompSVD.h>
#include <TMatrixD.h>
#include "Math/Vector3D.h"

namespace eicrecon {


  void FarDetectorMLReconstruction_factory::Init() {

    auto app = GetApplication();

    m_log = app->GetService<Log_service>()->logger(GetTag());

    auto cfg = GetDefaultConfig();

    japp->SetDefaultParameter(
            "lowq2:electron_energy",
            m_electron_beamE,
            "Electron beam energy [GeV]"
    );
    m_reco_algo.init();

  }


  void FarDetectorMLReconstruction_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    // Nothing to do here
  }

  void FarDetectorMLReconstruction_factory::Process(const std::shared_ptr<const JEvent> &event) {

    auto inputtracks = static_cast<const edm4eic::TrackParametersCollection*>(event->GetCollectionBase(GetDefaultInputTags()[0]));

    try {
      auto outputTracks = m_reco_algo.produce(*inputtracks);
      SetCollection(std::move(outputTracks));
    }
    catch(std::exception &e) {
      throw JException(e.what());
    }

  }

}
