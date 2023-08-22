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

    m_log = app->GetService<Log_service>()->logger(m_output_tag);

    // Create a set of variables and declare them to the reader
    // - the variable names MUST corresponds in name and type to those given in the weight file(s) used
    m_reader.AddVariable( "FarDetectorMLTracks[0].loc.a", &nnInput[FarDetectorMLNNIndexIn::PosY] );
    m_reader.AddVariable( "FarDetectorMLTracks[0].loc.b", &nnInput[FarDetectorMLNNIndexIn::PosZ] );
    m_reader.AddVariable( "sin(FarDetectorMLTracks[0].phi)*sin(FarDetectorMLTracks[0].theta)", &nnInput[FarDetectorMLNNIndexIn::DirX] );
    m_reader.AddVariable( "cos(FarDetectorMLTracks[0].phi)*sin(FarDetectorMLTracks[0].theta)", &nnInput[FarDetectorMLNNIndexIn::DirY] );

    // Locate and load the weight file
    // TODO - Add functionality to select passed by configiuration
    bool methodFound = false;
    const char* env_p = getenv(m_environment_path.c_str());
    if (env_p) {

	std::string dir_path;
        std::stringstream envvar_ss(env_p);
        while (getline(envvar_ss, dir_path, ':')) {
	  std::cout << dir_path << std::endl;
	    std::string weightName = dir_path +"/"+ m_file_path;
	  std::cout << weightName << std::endl;
	    if (std::filesystem::exists(weightName)){
   		try{
      		    m_method = dynamic_cast<TMVA::MethodBase*>(m_reader.BookMVA( m_method_name, weightName ));
    		}
    		catch(std::exception &e){
      		    throw JException(fmt::format("Failed to load method {} from file {}",m_method_name,weightName));
    		}
		methodFound = true;
		break;
	    }
        }
	if(!methodFound){
	  throw JException(fmt::format("File {} not found in any {} paths",m_file_path,m_environment_path));
	}

    }
    else {
      throw JException(fmt::format("Environment variable {} not found",m_environment_path));
    }

    japp->SetDefaultParameter(
            "lowq2:electron_energy",
            m_electron_beamE,
            "Electron beam energy [GeV]"
    );

  }


  void FarDetectorMLReconstruction_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    // Nothing to do here
  }

  void FarDetectorMLReconstruction_factory::Process(const std::shared_ptr<const JEvent> &event) {

    auto inputtracks =  event->Get<edm4eic::TrackParameters>(m_input_tag);

    std::vector<edm4eic::TrackParameters*> outputFarDetectorMLParticles(inputtracks.size());

    // Reconstructed particle members which don't change
    std::int32_t type   = 0; // Check?
    std::int32_t PDG    = 11;
    float        charge = -1;

    // Reconstructed particle members which don't change yet
    float             goodnessOfPID = 1;
    edm4hep::Vector3f referencePoint(0,0,0);
    edm4eic::Cov4f    covMatrix;
    float             mass = m_electron;

    uint ipart = 0;
    for(auto track: inputtracks){

      auto pos        = track->getLoc();
      auto trackphi   = track->getPhi();
      auto tracktheta = track->getTheta();

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
      edm4hep::Vector2f loc(0,0); //Temp unit transform
      // Point Error
      edm4eic::Cov2f locError;
      float theta   = momentum.Theta();;
      float phi     = momentum.Phi();
      float charge  = -1;
      float qOverP  = charge/sqrt(momMag2);
      edm4eic::Cov3f momentumError;
      float time  = 0;
      float timeError = 0;

      auto particle = new edm4eic::TrackParameters(type,loc,locError,theta,phi,qOverP,momentumError,time,timeError,charge);

      outputFarDetectorMLParticles[ipart++] = particle;

    }

    Set(std::move(outputFarDetectorMLParticles));

  }

}
