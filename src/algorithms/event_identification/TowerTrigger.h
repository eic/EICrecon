#pragma once
#include "TowerTriggerConfig.h"
#include "TriggerAlgorithm.h"

#include <string_view>
#include <algorithms/algorithm.h>
#include <algorithms/interfaces/WithPodConfig.h>

namespace eicrecon {


class TowerTrigger : public TriggerAlgorithm,
                     public WithPodConfig<TowerTriggerConfig> {

private:
    // member variables go here
    // access parameters via config()

public:
    TowerTrigger(std::string_view name);
    void init() final;
    void process(const Input& inputs, const Output& outputs) const final;

};

} // namespace
