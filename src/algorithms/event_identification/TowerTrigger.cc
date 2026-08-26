
#include "TowerTrigger.h"
#include "algorithms/event_identification/TriggerAlgorithm.h"
namespace eicrecon {

TowerTrigger::TowerTrigger(std::string_view name)
: TriggerAlgorithm(name,
            {}, // input names
            {}, // output names
            "Perform event identifiction using towers")
, WithPodConfig<TowerTriggerConfig>{}
{}

void TowerTrigger::init() {

}
void TowerTrigger::process(const Input& inputs, const Output& outputs) const {

}
} // namespace
