// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_CKFTracking_factory_H
#define EICRECON_CKFTracking_factory_H

#include <spdlog/spdlog.h>

#include <algorithms/tracking/TruthTrackSeeding.h>
#include "TrackSourceLinkerResult.h"
#include "GeoSvc.h"

#include "extensions/jana/JChainFactoryT.h"


namespace eicrecon {

    class CKFTracking_factory : public JChainFactoryT<TrackSourceLinkerResult> {

    public:
        CKFTracking_factory( std::vector<std::string> default_input_tags ):
                JChainFactoryT<TrackSourceLinkerResult>( std::move(default_input_tags) ) {
        }

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:

        std::shared_ptr<spdlog::logger> m_log;              /// Logger for this factory

        int m_verbose;                                      /// verbosity 0-none, 1-default, 2-debug, 3-trace
        std::string m_input_tag;                            /// Tag for the input data
        GeoSvc *m_acts_context;

    };

} // eicrecon

#endif //EICRECON_CKFTracking_factory_H
