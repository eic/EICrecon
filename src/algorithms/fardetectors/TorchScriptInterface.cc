#include "TorchScriptInterface.h"
#include "algorithms/fardetectors/TorchScriptInterfaceConfig.h"
#ifdef ClassDef
#undef ClassDef
#endif
#include <torch/script.h>

void eicrecon::TorchScriptInterface::init(std::shared_ptr<spdlog::logger> logger) {
  m_log = logger;
}

std::unique_ptr<edm4eic::ReconstructedParticleCollection> eicrecon::TorchScriptInterface::execute(const edm4eic::TrackerHitCollection *rchits) const{

  auto reconstructed_particles = std::make_unique<edm4eic::ReconstructedParticleCollection>();
  return reconstructed_particles;

}
