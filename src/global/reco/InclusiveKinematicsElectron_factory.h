// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <typeindex>
#include <utility>
#include <vector>

#include "algorithms/reco/InclusiveKinematicsElectron.h"
#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"

namespace eicrecon {

    class InclusiveKinematicsElectron_factory :
            public JChainMultifactoryT<>,
            public SpdlogMixin {

    public:

        explicit InclusiveKinematicsElectron_factory(
            std::string tag,
            const std::vector<std::string>& input_tags,
            const std::vector<std::string>& output_tags)
        : JChainMultifactoryT<>(std::move(tag), input_tags, output_tags) {

            DeclarePodioOutput<edm4eic::InclusiveKinematics>(GetOutputTags()[0]);

        }

        /** One time initialization **/
        void Init() override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    protected:
        InclusiveKinematicsElectron m_inclusive_kinematics_algo;

    };

} // eicrecon
