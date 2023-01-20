// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck

#pragma once

#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>
#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/InclusiveKinematics.h>
#include <algorithms/reco/InclusiveKinematicsElectron.h>


namespace eicrecon {

    class InclusiveKinematicsElectron_factory :
            public JChainFactoryT<ParticlesWithAssociation>,
            public SpdlogMixin<InclusiveKinematicsElectron_factory> {

    public:
        explicit InclusiveKinematicsElectron_factory(std::vector<std::string> default_input_tags):
            JChainFactoryT<ParticlesWithAssociation>(std::move(default_input_tags)) {
        }

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;
    protected:

        std::vector<std::string> m_input_assoc_tags = {"InclusiveKinematicsElectron"};
        InclusiveKinematicsElectron m_inclusive_kinematics_algo;

    };

} // eicrecon
