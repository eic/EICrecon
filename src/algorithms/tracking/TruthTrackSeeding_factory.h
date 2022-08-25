// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_TruthTrackSeeding_factory_H
#define EICRECON_TruthTrackSeeding_factory_H

#include <spdlog/spdlog.h>
#include <algorithms/interfaces/JChainFactoryT.h>

#include <algorithms/tracking/TruthTrackSeeding.h>



namespace eicrecon {

    class TruthTrackSeeding_factory : public JChainFactoryT<TrackSourceLinkerResult> {

    public:
        TruthTrackSeeding_factory( std::vector<std::string> default_input_tags ):
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

        eicrecon
    };

} // eicrecon

#endif //EICRECON_TruthTrackSeeding_factory_H
