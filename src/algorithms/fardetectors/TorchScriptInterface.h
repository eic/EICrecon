#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <spdlog/logger.h>
#include <memory>

#include "TorchScriptInterfaceConfig.h"
#include "algorithm/interfaces/WithPodConfig.h"

namespace eicrecon {

  class TorchScriptInterface : public WithPodConfig<TorchScriptInterfaceConfig> {

  public:

    void init(const dd4hep::Detector* det, const dd4hep::rec::CellIDPositionConverter* id_conv, std::shared_ptr<spdlog::logger> &logger);

    std::unique_ptr<edm4eic::ReconstructedParticleCollection> process(const edm4hep::SimTrackerHitCollection &inputhits);

  private:

    std::shared_ptr<spdlog::logger>   m_log;
    const dd4hep::Detector* m_detector{nullptr};
    const dd4hep::rec::CellIDPositionConverter* m_converter{nullptr};

  };
}
