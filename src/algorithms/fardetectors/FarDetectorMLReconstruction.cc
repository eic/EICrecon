
// Created by Simon Gardner to do FarDetectorML Tagger reconstruction
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3f.h>
#include <edm4eic/Cov4f.h>
#include <filesystem>

#include "FarDetectorMLReconstruction.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include <ROOT/RVec.hxx>
#include <TDecompSVD.h>
#include <TMatrixD.h>
#include "Math/Vector3D.h"

namespace eicrecon {


  void FarDetectorMLReconstruction::init() {

    // Create a set of variables and declare them to the reader
    // - the variable names MUST corresponds in name and type to those given in the weight file(s) used
    m_reader.AddVariable( "LowQ2Tracks[0].loc.a", &nnInput[FarDetectorMLNNIndexIn::PosY] );
    m_reader.AddVariable( "LowQ2Tracks[0].loc.b", &nnInput[FarDetectorMLNNIndexIn::PosZ] );
    m_reader.AddVariable( "sin(LowQ2Tracks[0].phi)*sin(LowQ2Tracks[0].theta)", &nnInput[FarDetectorMLNNIndexIn::DirX] );
    m_reader.AddVariable( "cos(LowQ2Tracks[0].phi)*sin(LowQ2Tracks[0].theta)", &nnInput[FarDetectorMLNNIndexIn::DirY] );

    // Locate and load the weight file
    // TODO - Add functionality to select passed by configuration
    bool methodFound = false;
    if(!m_cfg.modelPath.empty()){
      try{
        m_method = dynamic_cast<TMVA::MethodBase*>(m_reader.BookMVA( m_cfg.methodName, m_cfg.modelPath ));
      }
      catch(std::exception &e){
        throw JException(fmt::format("Failed to load method {} from file {}",m_cfg.methodName,m_cfg.modelPath));
      }

    }
    else{
      const char* env_p = getenv(m_cfg.environmentPath.c_str());
      if (env_p) {

        std::string dir_path;
        std::stringstream envvar_ss(env_p);
        while (getline(envvar_ss, dir_path, ':')) {
          std::cout << dir_path << std::endl;
          std::string weightName = dir_path +"/"+ m_cfg.fileName;
          std::cout << weightName << std::endl;
          if (std::filesystem::exists(weightName)){
            try{
              m_method = dynamic_cast<TMVA::MethodBase*>(m_reader.BookMVA( m_cfg.methodName, weightName ));
            }
            catch(std::exception &e){
              throw JException(fmt::format("Failed to load method {} from file {}",m_cfg.methodName,weightName));
            }
            methodFound = true;
            break;
          }
        }
        if(!methodFound){
          throw JException(fmt::format("File {} not found in any {} paths",m_cfg.fileName,m_cfg.environmentPath));
        }

      }
      else {
        throw JException(fmt::format("Environment variable {} not found",m_cfg.environmentPath));
      }
    }
  }


    std::tuple<
      std::unique_ptr<edm4eic::TrajectoryCollection>,
      std::unique_ptr<edm4eic::TrackParametersCollection>
    >
    FarDetectorMLReconstruction::produce(const edm4eic::TrackParametersCollection &inputtracks) {

    //TODO - Output would be the same size as input so memory handling could be better...
    auto outputFarDetectorMLTrajectories    = std::make_unique<edm4eic::TrajectoryCollection>();
    auto outputFarDetectorMLTrackParameters = std::make_unique<edm4eic::TrackParametersCollection>();

    // Reconstructed particle members which don't change
    std::int32_t type   = 0; // Check?
    std::int32_t PDG    = 11;
    float        charge = -1;

    // Reconstructed particle members which don't change yet
    float             goodnessOfPID = 1;
    edm4hep::Vector3f referencePoint(0,0,0);
    edm4eic::Cov4f    covMatrix;
    float             mass = m_electron;

    for(auto track: inputtracks){

      auto pos        = track.getLoc();
      auto trackphi   = track.getPhi();
      auto tracktheta = track.getTheta();

      nnInput[FarDetectorMLNNIndexIn::PosY] = pos.a;
      nnInput[FarDetectorMLNNIndexIn::PosZ] = pos.b;
      nnInput[FarDetectorMLNNIndexIn::DirX] = sin(trackphi)*sin(tracktheta);
      nnInput[FarDetectorMLNNIndexIn::DirY] = cos(trackphi)*sin(tracktheta);

      auto values = m_method->GetRegressionValues();

      ROOT::Math::XYZVector momentum = ROOT::Math::XYZVector(values[FarDetectorMLNNIndexOut::MomX]*m_electron_beamE,values[FarDetectorMLNNIndexOut::MomY]*m_electron_beamE,values[FarDetectorMLNNIndexOut::MomZ]*m_electron_beamE);

      float momMag2 = values[FarDetectorMLNNIndexOut::MomX]*m_electron_beamE*values[FarDetectorMLNNIndexOut::MomX]*m_electron_beamE+
        values[FarDetectorMLNNIndexOut::MomY]*m_electron_beamE*values[FarDetectorMLNNIndexOut::MomY]*m_electron_beamE+
        values[FarDetectorMLNNIndexOut::MomZ]*m_electron_beamE*values[FarDetectorMLNNIndexOut::MomZ]*m_electron_beamE;

      float energy = sqrt(momMag2+mass*mass);

      // Track parameter variables
      // TODO: Add time and momentum errors
      int type = 0;
      // Plane Point
      edm4hep::Vector2f loc(0,0); // Vertex estimate
      // Point Error
      edm4eic::Cov2f locError;
      float theta   = momentum.Theta();;
      float phi     = momentum.Phi();
      float charge  = -1;
      float qOverP  = charge/sqrt(momMag2);
      edm4eic::Cov3f momentumError;
      float time  = 0;
      float timeError = 0;

      edm4eic::TrackParameters params =  outputFarDetectorMLTrackParameters->create(type,loc,locError,theta,phi,qOverP,momentumError,time,timeError,charge);

      auto trajectory = outputFarDetectorMLTrajectories->create();
      trajectory.addToTrackParameters(params);

    }

    return std::make_tuple( std::move(outputFarDetectorMLTrajectories), std::move(outputFarDetectorMLTrackParameters));

  }

}
