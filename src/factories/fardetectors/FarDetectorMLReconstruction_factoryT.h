// Created by Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <spdlog/spdlog.h>

// Event Model related classes
#include <edm4eic/TrajectoryCollection.h>
#include <algorithms/fardetectors/FarDetectorMLReconstruction.h>
#include <algorithms/fardetectors/FarDetectorMLReconstructionConfig.h>

#include <extensions/jana/JChainMultifactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>
#include <Evaluator/DD4hepUnits.h>

namespace eicrecon {

  class FarDetectorMLReconstruction_factoryT :
  public JChainMultifactoryT<FarDetectorMLReconstructionConfig>,
  public SpdlogMixin {

  public:

    explicit FarDetectorMLReconstruction_factoryT(
      std::string tag,
      const std::vector<std::string>& input_tags,
      const std::vector<std::string>& output_tags,
      FarDetectorMLReconstructionConfig cfg ):
      JChainMultifactoryT<FarDetectorMLReconstructionConfig>(std::move(tag), input_tags, output_tags, cfg) {

         DeclarePodioOutput<edm4eic::Trajectory>(GetOutputTags()[0]);
         DeclarePodioOutput<edm4eic::TrackParameters>(GetOutputTags()[1]);

      }

      /** One time initialization **/
      void Init() override {

        auto app = GetApplication();

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(app, GetPrefix(), "info");

        // This prefix will be used for parameters
        std::string param_prefix = GetPluginName() + ":" + GetTag();

        auto cfg = GetDefaultConfig();

        japp->SetDefaultParameter(param_prefix+":electron_beamE", cfg.electron_beamE,"Electron beam energy [GeV]"             );
        japp->SetDefaultParameter(param_prefix+":model_path",     cfg.modelPath,     "Path to the model file"                 );
        japp->SetDefaultParameter(param_prefix+":method_name",    cfg.methodName,    "Name of the method to use from the file");

        // Setup algorithm
        m_reco_algo.applyConfig(cfg);

        m_reco_algo.init();

      }

      /** Event by event processing **/
      void Process(const std::shared_ptr<const JEvent> &event) override {
        auto inputtracks = static_cast<const edm4eic::TrackParametersCollection*>(event->GetCollectionBase(GetInputTags()[0]));

        try {
          auto [outputTrajectories, outputTrackParameters] = m_reco_algo.produce(*inputtracks);
          SetCollection<edm4eic::Trajectory>     (GetOutputTags()[0], std::move(outputTrajectories)   );
          SetCollection<edm4eic::TrackParameters>(GetOutputTags()[1], std::move(outputTrackParameters));
        }
        catch(std::exception &e) {
          throw JException(e.what());
        }

      }

  private:
      eicrecon::FarDetectorMLReconstruction m_reco_algo;        // ML reconstruction fitting algorithm

  };

} // eicrecon
