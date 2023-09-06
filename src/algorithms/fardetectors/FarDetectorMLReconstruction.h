// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner

#pragma once

// Event Model related classes
#include <edm4eic/TrajectoryCollection.h>
#include <edm4eic/TrackParametersCollection.h>

#include <Evaluator/DD4hepUnits.h>
#include <TMVA/MethodBase.h>
#include <TMVA/Reader.h>
#include "algorithms/interfaces/WithPodConfig.h"
#include "FarDetectorMLReconstructionConfig.h"


namespace eicrecon {

  enum FarDetectorMLNNIndexIn{PosY,PosZ,DirX,DirY};
  enum FarDetectorMLNNIndexOut{MomX,MomY,MomZ};

  class FarDetectorMLReconstruction : public WithPodConfig<FarDetectorMLReconstructionConfig> {

  public:

      /** One time initialization **/
      void init();

      /** Event by event processing **/
      std::tuple<
        std::unique_ptr<edm4eic::TrajectoryCollection>,
        std::unique_ptr<edm4eic::TrackParametersCollection>
      >
      produce(const edm4eic::TrackParametersCollection &inputtracks);

      //----- Define constants here ------

  private:
      TMVA::Reader          m_reader{"!Color:!Silent"};
      TMVA::MethodBase*     m_method{nullptr};
      float nnInput[4]      = {0.0,0.0,0.0,0.0};

  };

} // eicrecon
