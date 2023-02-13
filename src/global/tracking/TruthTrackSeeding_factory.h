// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <spdlog/spdlog.h>
#include "extensions/jana/JChainFactoryT.h"

#include <algorithms/tracking/TruthTrackSeeding.h>





namespace eicrecon {

class TruthTrackSeeding_factory : public JChainFactoryT<edm4eic::TrackParameters> {

    public:
        TruthTrackSeeding_factory( std::vector<std::string> default_input_tags ):
                JChainFactoryT<edm4eic::TrackParameters>( std::move(default_input_tags) ) {
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
        std::vector<std::string> m_input_tags;              /// Tag for the input data

        eicrecon::TruthTrackSeeding m_truth_track_seeding_algo;  /// Truth track seeding algorithm

    };

} // eicrecon
