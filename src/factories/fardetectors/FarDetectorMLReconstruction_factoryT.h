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

  class FarDetectorMLReconstruction_factoryT : public JChainMultifactoryT<FarDetectorMLReconstructionConfig>{

  public:

    explicit FarDetectorMLReconstruction_factoryT(
      std::string tag,
      const std::vector<std::string>& input_tags,
      const std::vector<std::string>& output_tags,
      FarDetectorMLReconstructionConfig cfg ):
      JChainMultifactoryT<FarDetectorMLReconstructionConfig>(std::move(tag), input_tags, output_tags, cfg) {

	 DeclarePodioOutput<edm4eic::Trajectory>(GetOutputTags()[0]);

      } //constructer

      /** One time initialization **/
      void Init() override {

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

      /** Event by event processing **/
      void Process(const std::shared_ptr<const JEvent> &event) override {
	auto inputtracks = static_cast<const edm4eic::TrackParametersCollection*>(event->GetCollectionBase(GetInputTags()[0]));

	try {
	  auto outputTracks = m_reco_algo.produce(*inputtracks);
	  SetCollection<edm4eic::Trajectory>(GetOutputTags()[0],std::move(outputTracks));
	}
	catch(std::exception &e) {
	  throw JException(e.what());
	}

      }

  private:
      std::shared_ptr<spdlog::logger>       m_log;              // Logger for this factory
      eicrecon::FarDetectorMLReconstruction m_reco_algo;        // ML reconstruction fitting algorithm

      float m_electron_beamE = 10*dd4hep::GeV;

      float m_electron{0.000510998928}; //TODO: Link to constant elsewhere?

      // Stuff to add to config
      std::string m_method_name{"DNN_CPU"};
      std::string m_file_path{"LowQ2_DNN_CPU.weights.xml"};
      std::string m_environment_path{"JANA_PLUGIN_PATH"};

  };

} // eicrecon
