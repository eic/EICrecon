// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <spdlog/spdlog.h>

#include <edm4eic/TrackParametersCollection.h>

#include "algorithms/tracking/CKFTracking.h"
#include "algorithms/tracking/CKFTrackingConfig.h"

#include "extensions/spdlog/SpdlogMixin.h"
#include "extensions/jana/JChainMultifactoryT.h"

namespace eicrecon {

    class CKFTracking_factory :
            public JChainMultifactoryT<CKFTrackingConfig>,
            public SpdlogMixin {

    public:

        explicit CKFTracking_factory(
            std::string tag,
            const std::vector<std::string>& input_tags,
            const std::vector<std::string>& output_tags,
            CKFTrackingConfig cfg)
        : JChainMultifactoryT<CKFTrackingConfig>(std::move(tag), input_tags, output_tags, cfg) {

            DeclarePodioOutput<edm4eic::Trajectory>(GetOutputTags()[0]);
            DeclarePodioOutput<edm4eic::TrackParameters>(GetOutputTags()[1]);
            DeclareOutput<ActsExamples::Trajectories>(GetOutputTags()[2]);

        }

        /** One time initialization **/
        void Init() override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:

        CKFTracking m_tracking_algo;                      /// Proxy tracking algorithm

    };

} // eicrecon
