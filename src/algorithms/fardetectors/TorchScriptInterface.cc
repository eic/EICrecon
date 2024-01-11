#include "TorchScriptInterface.h"
#include "algorithms/fardetectors/TorchScriptInterfaceConfig.h"
#undef ClassDef
#include <torch/script.h>



void eicrecon::TorchScriptInterface::init(const dd4hep::Detector* det,
                                          const dd4hep::rec::CellIDPositionConverter* id_conv,
                                          std::shared_ptr<spdlog::logger> &logger) {
  m_log       = logger;
  m_detector  = det;
  m_converter = id_conv;
 
}

std::unique_ptr<edm4eic::ReconstructedParticleCollection> eicrecon::TorchScriptInterface::process(const edm4hep::SimTrackerHitCollection&){
  

  return 0;

} 

