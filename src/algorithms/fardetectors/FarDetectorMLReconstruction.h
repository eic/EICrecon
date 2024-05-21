// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner

#pragma once

// Event Model related classes
#include <edm4eic/TrajectoryCollection.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrackCollection.h>

#include <algorithms/algorithm.h>
#include <Evaluator/DD4hepUnits.h>
#include <TMVA/MethodBase.h>
#include <TMVA/Reader.h>
#include "algorithms/interfaces/WithPodConfig.h"
#include "FarDetectorMLReconstructionConfig.h"

#include <spdlog/logger.h>


namespace eicrecon {

  enum FarDetectorMLNNIndexIn{PosY,PosZ,DirX,DirY};
  enum FarDetectorMLNNIndexOut{MomX,MomY,MomZ};
  
  using FarDetectorMLReconstructionAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      edm4eic::TrackParametersCollection
    >,
    algorithms::Output<
      edm4eic::TrajectoryCollection,
      edm4eic::TrackParametersCollection,
      edm4eic::TrackCollection
    >
  >;

  class FarDetectorMLReconstruction
  : public FarDetectorMLReconstructionAlgorithm,
    public WithPodConfig<FarDetectorMLReconstructionConfig> {

  public:
      FarDetectorMLReconstruction(std::string_view name)
        : FarDetectorMLReconstructionAlgorithm{name,
                              {"TrackParameters"},
                              {"Trajectory","TrackParameters","Track"},
                              "Reconstruct track parameters using ML method."} {}


      /** One time initialization **/
      void init();

      /** Event by event processing **/
      void process(const Input&, const Output&);

      //----- Define constants here ------

  private:
      TMVA::Reader          m_reader{"!Color:!Silent"};
      TMVA::MethodBase*     m_method{nullptr};
      float nnInput[4]      = {0.0,0.0,0.0,0.0};

  };

} // eicrecon
