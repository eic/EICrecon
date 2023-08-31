// Created by Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <spdlog/spdlog.h>

// Event Model related classes
#include <edm4eic/TrackParametersCollection.h>

#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>
#include <Evaluator/DD4hepUnits.h>
#include <TMVA/MethodBase.h>
#include <TMVA/Reader.h>
#include "algorithms/interfaces/WithPodConfig.h"
#include "FarDetectorMLReconstructionConfig.h"


namespace eicrecon {

  //  enum FarDetectorMLNNIndex{Energy,Theta,X,Y};
  enum FarDetectorMLNNIndexIn{PosY,PosZ,DirX,DirY};
  enum FarDetectorMLNNIndexOut{MomX,MomY,MomZ};

  class FarDetectorMLReconstruction : public WithPodConfig<FarDetectorMLReconstructionConfig> {

  public:

      /** One time initialization **/
      void init();

      /** Event by event processing **/
      std::unique_ptr<edm4eic::TrackParametersCollection> produce(const edm4eic::TrackParametersCollection &inputtracks);

      //----- Define constants here ------

  private:
      TMVA::Reader          m_reader{"!Color:!Silent"};
      TMVA::MethodBase*     m_method{nullptr};
      float nnInput[4];

      float m_electron_beamE = 10*dd4hep::GeV;

      float m_electron{0.000510998928}; //TODO: Link to constant elsewhere?

      // Stuff to add to config
      std::string m_method_name{"DNN_CPU"};
      std::string m_file_path{"LowQ2_DNN_CPU.weights.xml"};
      std::string m_environment_path{"JANA_PLUGIN_PATH"};

  };

} // eicrecon
