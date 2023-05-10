// Created by Simon Gardner to do LowQ2 Tagger reconstruction
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include "LowQ2Reconstruction_factory.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "extensions/string/StringHelpers.h"

namespace eicrecon {
  
  
  LowQ2Reconstruction_factory::LowQ2Reconstruction_factory(){ SetTag(m_output_tag); }
  
  void LowQ2Reconstruction_factory::Init() {
    
    std::string plugin_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");
    std::string param_prefix = plugin_name + ":" + m_input_tag + ":";
    
    auto app = GetApplication();
    
    m_log = app->GetService<Log_service>()->logger(m_output_tag);
    
    
    reader = new TMVA::Reader( "!Color:!Silent" );
    
    // Create a set of variables and declare them to the reader
    // - the variable names MUST corresponds in name and type to those given in the weight file(s) used
    Float_t yP, zP, xV, yV;
    
    reader->AddVariable( "real_cut[0].fCoordinates.fY",    &yP     );
    reader->AddVariable( "real_cut[0].fCoordinates.fZ",    &zP     );
    reader->AddVariable( "real_vector[0].fCoordinates.fX", &xV     );
    reader->AddVariable( "real_vector[0].fCoordinates.fY", &yV     );
    
    // Spectator variables declared in the training have to be added to the reader, too
    Float_t  eE, logQ2;
    reader->AddSpectator( "eE",     &eE );
    reader->AddSpectator( "logQ2",  &logQ2 );
    
    reader->BookMVA( methodName, weightfile );
    //method = dynamic_cast<TMVA::MethodBase*>(reader->FindMVA(methodName));
    
  }
  
  


  void LowQ2Reconstruction_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    // Nothing to do here
  }
  
  void LowQ2Reconstruction_factory::Process(const std::shared_ptr<const JEvent> &event) {
    
    
    std::vector<edm4eic::ReconstructedParticle*> outputLowQ2Particles;
    
    auto inputtracks =  event->Get<edm4eic::TrackParameters>(m_input_tag);
    
    for(auto track: inputtracks){

      auto pos      = track->getLoc();
      auto trackphi = track->getPhi();

      *yP = pos.a;
      *zP = pos.b;
      *xV = sin(trackphi);
      *yV = cos(trackphi);
      auto values = reader->EvaluateRegression(methodName);

      auto origintheta = values[1];
      auto originphi   = atan2(values[3],values[2]);
            
      std::int32_t type;
      float energy = values[0];
      edm4hep::Vector3f momentum(values[3],values[2],cos(values[1]));
      edm4hep::Vector3f referencePoint;
      float charge = -1;
      float mass;
      float goodnessOfPID = 1;
      edm4eic::Cov4f covMatrix;
      std::int32_t PDG = 11;
      
      auto particle = new edm4eic::ReconstructedParticle(type,energy,momentum,referencePoint,charge,mass,goodnessOfPID,covMatrix,PDG);
      
      outputLowQ2Particles.push_back(particle);
      
    }
    
    Set(outputLowQ2Particles);
    
  }
  
}
