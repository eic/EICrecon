// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <spdlog/spdlog.h>

#include <algorithms/tracking/CKFTracking.h>
#include <algorithms/tracking/CKFTrackingConfig.h>
#include <algorithms/tracking/TrackerSourceLinkerResult.h>

#include <extensions/spdlog/SpdlogMixin.h>
#include <extensions/jana/JChainFactoryT.h>


namespace eicrecon {

    class CKFTracking_factory :
            public JChainFactoryT<eicrecon::TrackingResultTrajectory, CKFTrackingConfig, JFactoryT>,
            public SpdlogMixin<CKFTracking_factory> {

    public:
        CKFTracking_factory( std::vector<std::string> default_input_tags, CKFTrackingConfig cfg):
                JChainFactoryT<eicrecon::TrackingResultTrajectory, CKFTrackingConfig, JFactoryT>(std::move(default_input_tags), cfg ) {
        }

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:

        CKFTracking m_tracking_algo;                      /// Proxy tracking algorithm

    };

} // eicrecon
