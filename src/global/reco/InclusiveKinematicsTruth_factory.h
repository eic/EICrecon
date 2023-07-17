// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck

#pragma once

#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <algorithms/reco/InclusiveKinematicsTruth.h>

namespace eicrecon {

    class InclusiveKinematicsTruth_factory :
            public JChainFactoryT<edm4eic::InclusiveKinematics>,
            public SpdlogMixin<InclusiveKinematicsTruth_factory> {

    public:
        explicit InclusiveKinematicsTruth_factory(std::vector<std::string> default_input_tags):
            JChainFactoryT<edm4eic::InclusiveKinematics>(std::move(default_input_tags)) {
        }

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;
    protected:

        std::vector<std::string> m_input_assoc_tags = {"InclusiveKinematicsTruth"};
        InclusiveKinematicsTruth m_inclusive_kinematics_algo;

    };

} // eicrecon
