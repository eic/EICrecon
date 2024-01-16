#include <gsl/pointers>
#include "TorchScriptInterface.h"
#include "algorithms/fardetectors/TorchScriptInterfaceConfig.h"
#ifdef ClassDef
#undef ClassDef
#endif
#include <torch/script.h>



void eicrecon::TorchScriptInterface::init(const dd4hep::Detector* det,
                                          const dd4hep::rec::CellIDPositionConverter* id_conv,
                                          std::shared_ptr<spdlog::logger> &logger) {
  m_log       = logger;
  m_detector  = det;
  m_converter = id_conv;

}

void eicrecon::TorchScriptInterface::process(const Input& input,
   					     const Output& output) const {

  return;

}
