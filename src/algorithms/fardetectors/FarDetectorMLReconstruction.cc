// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3f.h>
#include <edm4eic/Cov4f.h>
#include <edm4eic/vector_utils.h>
#include <edm4hep/utils/vector_utils.h>

#include <filesystem>

#include "FarDetectorMLReconstruction.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"

namespace eicrecon {


  void FarDetectorMLReconstruction::init() {

   m_reader = new TMVA::Reader( "!Color:!Silent" );
    // Create a set of variables and declare them to the reader
    // - the variable names MUST corresponds in name and type to those given in the weight file(s) used
    m_reader->AddVariable( "LowQ2Tracks[0].loc.a", &nnInput[FarDetectorMLNNIndexIn::PosY] );
    m_reader->AddVariable( "LowQ2Tracks[0].loc.b", &nnInput[FarDetectorMLNNIndexIn::PosZ] );
    m_reader->AddVariable( "sin(LowQ2Tracks[0].phi)*sin(LowQ2Tracks[0].theta)", &nnInput[FarDetectorMLNNIndexIn::DirX] );
    m_reader->AddVariable( "cos(LowQ2Tracks[0].phi)*sin(LowQ2Tracks[0].theta)", &nnInput[FarDetectorMLNNIndexIn::DirY] );

    // Locate and load the weight file
    // TODO - Add functionality to select passed by configuration
    bool methodFound = false;
    if(!m_cfg.modelPath.empty()){
      try{
        m_method = dynamic_cast<TMVA::MethodBase*>(m_reader->BookMVA( m_cfg.methodName, m_cfg.modelPath ));
      }
      catch(std::exception &e){
        error(fmt::format("Failed to load method {} from file {}",m_cfg.methodName,m_cfg.modelPath));
      }

    } else {
      error("No model path provided for FarDetectorMLReconstruction");
    }
  }



  void FarDetectorMLReconstruction::process(
      const FarDetectorMLReconstruction::Input& input,
      const FarDetectorMLReconstruction::Output& output) {

    const auto [inputTracks] = input;
    auto [outputFarDetectorMLTrajectories, outputFarDetectorMLTrackParameters, outputFarDetectorMLTracks] = output;

    // Reconstructed particle members which don't change
    std::int32_t type   = 0; // Check?
    float        charge = -1;

    for(const auto& track: *inputTracks){

      auto pos        = track.getLoc();
      auto trackphi   = track.getPhi();
      auto tracktheta = track.getTheta();

      nnInput[FarDetectorMLNNIndexIn::PosY] = pos.a;
      nnInput[FarDetectorMLNNIndexIn::PosZ] = pos.b;
      nnInput[FarDetectorMLNNIndexIn::DirX] = sin(trackphi)*sin(tracktheta);
      nnInput[FarDetectorMLNNIndexIn::DirY] = cos(trackphi)*sin(tracktheta);

      auto values = m_method->GetRegressionValues();

      edm4hep::Vector3f momentum = {values[FarDetectorMLNNIndexOut::MomX],values[FarDetectorMLNNIndexOut::MomY],values[FarDetectorMLNNIndexOut::MomZ]};

      // log out the momentum components and magnitude
      debug("Prescaled Output Momentum: x {}, y {}, z {}",values[FarDetectorMLNNIndexOut::MomX],values[FarDetectorMLNNIndexOut::MomY],values[FarDetectorMLNNIndexOut::MomZ]);
      debug("Prescaled Momentum: {}",edm4eic::magnitude(momentum));

      // Scale momentum magnitude
      momentum = momentum*m_cfg.electronBeamE;
      debug("Scaled Momentum: {}",edm4eic::magnitude(momentum));

      // Track parameter variables
      // TODO: Add time and momentum errors
      // Plane Point
      edm4hep::Vector2f loc(0,0); // Vertex estimate
      uint64_t surface = 0; //Not used in this context
      float theta   = edm4eic::anglePolar(momentum);
      float phi     = edm4eic::angleAzimuthal(momentum);
      float qOverP  = charge/edm4eic::magnitude(momentum);
      float time;
      // PDG
      int32_t pdg = 11;
      // Point Error
      edm4eic::Cov6f error;

      edm4eic::TrackParameters params =  outputFarDetectorMLTrackParameters->create(type,surface,loc,theta,phi,qOverP,time,pdg,error);

      auto trajectory = outputFarDetectorMLTrajectories->create();
      trajectory.addToTrackParameters(params);

      int32_t trackType = 0;
      edm4hep::Vector3f position = {0,0,0};
      float timeError;
      float charge    = -1;
      float chi2      = 0;
      uint32_t ndf    = 0;

      auto outTrack      = outputFarDetectorMLTracks->create(trackType,position,momentum,error,time,timeError,charge,chi2,ndf);
      outTrack.setTrajectory(trajectory);

    }

  }

}
