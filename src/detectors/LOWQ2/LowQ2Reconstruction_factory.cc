// Created by Simon Gardner to do LowQ2 Tagger reconstruction
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3f.h>
#include <edm4eic/Cov4f.h>

#include "LowQ2Reconstruction_factory.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include <ROOT/RVec.hxx>
#include <TDecompSVD.h>
#include <TMatrixD.h>
#include "Math/Vector3D.h"

namespace eicrecon {


  void LowQ2Reconstruction_factory::Init() {

    auto app = GetApplication();

    m_log = app->GetService<Log_service>()->logger(m_output_tag);

    auto weightDir = std::string(std::getenv( "EICrecon_ROOT" ));

    //std::string weightName = m_file_path;
    std::string weightName = weightDir + m_file_path;

    model = std::make_shared<TMVA_SOFIE_trainedTaggerRegressionModel::Session>(weightName);
    
  }




  void LowQ2Reconstruction_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    // Nothing to do here
  }

  void LowQ2Reconstruction_factory::Process(const std::shared_ptr<const JEvent> &event) {

    auto inputtracks =  event->Get<edm4eic::TrackParameters>(m_input_tag);

    //    std::vector<std::unique_ptr<edm4eic::ReconstructedParticle>> outputLowQ2Particles(inputtracks.size());
    std::vector<edm4eic::TrackParameters*> outputLowQ2Particles(inputtracks.size());
    //std::vector<edm4eic::ReconstructedParticle*> outputLowQ2Particles(inputtracks.size());

    // Reconstructed particle members which don't change
    std::int32_t type   = 0; // Check?
    std::int32_t PDG    = -11;
    float        charge = -1;

    // Reconstructed particle members which don't change yet
    float             goodnessOfPID = 1;
    edm4hep::Vector3f referencePoint(0,0,0);
    edm4eic::Cov4f    covMatrix;
    float             mass = m_electron;

    float nnInput[4];

    uint ipart = 0;
    for(auto track: inputtracks){

      auto pos        = track->getLoc();
      auto trackphi   = track->getPhi();
      auto tracktheta = track->getTheta();

      nnInput[LowQ2NNIndexIn::PosY] = pos.a;
      nnInput[LowQ2NNIndexIn::PosZ] = pos.b;
      nnInput[LowQ2NNIndexIn::DirX] = sin(trackphi)*sin(tracktheta);
      nnInput[LowQ2NNIndexIn::DirY] = cos(trackphi)*sin(tracktheta);
    
      
      //auto values = TMVA_SOFIE_trainedTaggerRegressionModel::infer(nnInput);
      auto values = model->infer(nnInput);
      //auto values = m_method->GetRegressionValues();

      ROOT::Math::XYZVector momentum = ROOT::Math::XYZVector(values[LowQ2NNIndexOut::MomX]*beamE,values[LowQ2NNIndexOut::MomY]*beamE,values[LowQ2NNIndexOut::MomZ]*beamE);
      //      edm4hep::Vector3f momentum(values[LowQ2NNIndex::MomX]*beamE,values[LowQ2NNIndex::MomY]*beamE,values[LowQ2NNIndex::MomZ]*beamE);
      float energy = sqrt(values[LowQ2NNIndexOut::MomX]*beamE*values[LowQ2NNIndexOut::MomX]*beamE+
			  values[LowQ2NNIndexOut::MomY]*beamE*values[LowQ2NNIndexOut::MomY]*beamE+
			  values[LowQ2NNIndexOut::MomZ]*beamE*values[LowQ2NNIndexOut::MomZ]*beamE
			  +mass*mass);

      float momMag2 = values[LowQ2NNIndexOut::MomX]*beamE*values[LowQ2NNIndexOut::MomX]*beamE+
	values[LowQ2NNIndexOut::MomY]*beamE*values[LowQ2NNIndexOut::MomY]*beamE+
	values[LowQ2NNIndexOut::MomZ]*beamE*values[LowQ2NNIndexOut::MomZ]*beamE;

      // Track parameter variables
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

//       //      auto particle = new edm4eic::ReconstructedParticle(type,energy,momentum,referencePoint,charge,mass,goodnessOfPID,covMatrix,PDG);
      auto particle = new edm4eic::TrackParameters(type,loc,locError,theta,phi,qOverP,momentumError,time,timeError,charge);

      outputLowQ2Particles[ipart++] = particle;

    }

    Set(std::move(outputLowQ2Particles));

  }

}
