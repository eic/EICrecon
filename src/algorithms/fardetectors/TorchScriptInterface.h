#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <string>
#include <string_view>

#include "TorchScriptInterfaceConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/algorithm.h"

namespace eicrecon {
  using TorchScriptInterfaceAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      edm4hep::SimTrackerHitCollection
    >,
    algorithms::Output<
      edm4eic::ReconstructedParticleCollection
    >
  >;

  class TorchScriptInterface : public TorchScriptInterfaceAlgorithm,
                               public WithPodConfig<TorchScriptInterfaceConfig> {

  public:
    void init(const dd4hep::Detector* detector, const dd4hep::rec::CellIDPositionConverter* id_conv, std::shared_ptr<spdlog::logger>& logger);
    void process(const Input&, const Output&) const final;

  private:

    std::shared_ptr<spdlog::logger>   m_log;
    const dd4hep::Detector* m_detector{nullptr};
    const dd4hep::rec::CellIDPositionConverter* m_converter{nullptr};

  };
}
