// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck

#pragma once

#include "extensions/jana/JChainFactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"
#include "algorithms/reco/InclusiveKinematicsElectron.h"

namespace eicrecon {

    class InclusiveKinematicsElectron_factory :
            public JChainFactoryT<edm4eic::InclusiveKinematics>,
            public SpdlogMixin<InclusiveKinematicsElectron_factory> {

    public:
        explicit InclusiveKinematicsElectron_factory(std::vector<std::string> default_input_tags):
            JChainFactoryT<edm4eic::InclusiveKinematics>(std::move(default_input_tags)) {
        }

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    protected:
        InclusiveKinematicsElectron m_inclusive_kinematics_algo;

    };

} // eicrecon
