// Created by Simon Gardner to do LowQ2 Tagger reconstruction
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include "LowQ2Reconstruction_factory.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"

namespace eicrecon {
   
  
  void LowQ2Reconstruction_factory::Init() {
    
    auto app = GetApplication();
    
    m_log = app->GetService<Log_service>()->logger(m_output_tag);
    
    // Create a set of variables and declare them to the reader
    // - the variable names MUST corresponds in name and type to those given in the weight file(s) used
    m_reader.AddVariable ( "real_cut[0].fCoordinates.fY",    &m_yP    );
    m_reader.AddVariable ( "real_cut[0].fCoordinates.fZ",    &m_zP    );
    m_reader.AddVariable ( "real_vector[0].fCoordinates.fX", &m_xV    );
    m_reader.AddVariable ( "real_vector[0].fCoordinates.fY", &m_yV    );
    
    // Spectator variables declared in the training have to be added to the reader, too
    m_reader.AddSpectator( "eE",                             &m_eE    );
    m_reader.AddSpectator( "logQ2",                          &m_logQ2 );
    
    m_reader.BookMVA( m_method_name, m_weight_file );
    m_method = dynamic_cast<TMVA::MethodBase*>(m_reader.FindMVA(m_method_name));

  }
  
  


  void LowQ2Reconstruction_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    // Nothing to do here
  }
  
  void LowQ2Reconstruction_factory::Process(const std::shared_ptr<const JEvent> &event) {
    
    auto inputtracks =  event->Get<edm4eic::TrackParameters>(m_input_tag);
    
    //    std::vector<std::unique_ptr<edm4eic::ReconstructedParticle>> outputLowQ2Particles(inputtracks.size());
    std::vector<edm4eic::ReconstructedParticle*> outputLowQ2Particles(inputtracks.size());

    // Reconstructed particle members which don't change
    std::int32_t type   = 0; // Check?
    std::int32_t PDG    = -11;
    float        charge = -1;

    // Reconstructed particle members which don't change yet
    float             goodnessOfPID = 1;
    edm4hep::Vector3f referencePoint(0,0,0);
    edm4eic::Cov4f    covMatrix;
    float             mass = m_electron;

    uint ipart = 0;
    for(auto track: inputtracks){

      auto pos      = track->getLoc();
      auto trackphi = track->getPhi();

      m_yP = pos.a;
      m_zP = pos.b;
      m_xV = sin(trackphi);
      m_yV = cos(trackphi);
      auto values = m_method->GetRegressionValues();
      
      float energy = values[LowQ2NNIndex::Energy];
      edm4hep::Vector3f momentum(values[LowQ2NNIndex::X],values[LowQ2NNIndex::Y],cos(values[LowQ2NNIndex::Theta]));
      
      auto particle = new edm4eic::ReconstructedParticle(type,energy,momentum,referencePoint,charge,mass,goodnessOfPID,covMatrix,PDG);
      
      outputLowQ2Particles[ipart++] = particle;
    
    }
    
    Set(std::move(outputLowQ2Particles));
    
  }
  
}
