// Created by Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <spdlog/spdlog.h>

// Event Model related classes
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>

#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>
#include <Evaluator/DD4hepUnits.h>
#include <TMVA/MethodBase.h>
#include <TMVA/Reader.h>

namespace eicrecon {

  //  enum LowQ2NNIndex{Energy,Theta,X,Y};
  enum LowQ2NNIndexIn{PosY,PosZ,DirX,DirY};
  enum LowQ2NNIndexOut{MomX,MomY,MomZ};

  class LowQ2Reconstruction_factory : public JChainFactoryT<edm4eic::TrackParameters, NoConfig>{

  public:

    LowQ2Reconstruction_factory( std::vector<std::string> default_input_tags):
      JChainFactoryT<edm4eic::TrackParameters, NoConfig>(std::move(default_input_tags) ) {
    }

      LowQ2Reconstruction_factory(); //constructer

      /** One time initialization **/
      void Init() override;

      /** On run change preparations **/
      void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

      /** Event by event processing **/
      void Process(const std::shared_ptr<const JEvent> &event) override;

      //----- Define constants here ------

  private:
      std::shared_ptr<spdlog::logger> m_log;              // Logger for this factory
      std::string m_input_tag{"LowQ2Tracks"};
      std::string m_output_tag{"LowQ2Particles"};

      TMVA::Reader          m_reader{"!Color:!Silent"};
      TMVA::MethodBase*     m_method{nullptr};
      float nnInput[4];

      float beamE = 10*dd4hep::GeV; //TODO: Change when this is included in metadata/other location/as config

      float m_electron{0.000510998928}; //TODO: Link to constant elsewhere?

      // Stuff to add to config
      std::string m_method_name{"DNN_CPU"};
      std::string m_file_path{"LowQ2_DNN_CPU.weights.xml"};
      std::string m_environment_path{"JANA_PLUGIN_PATH"};

  };

} // eicrecon
