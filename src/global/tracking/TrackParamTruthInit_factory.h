// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_TrackParamTruthInit_factory_H
#define EICRECON_TrackParamTruthInit_factory_H

#include <spdlog/spdlog.h>
#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>

#include <algorithms/tracking/TrackParamTruthInit.h>



namespace eicrecon {

class TrackParamTruthInit_factory :
        public JChainFactoryT<eicrecon::TrackParameters, TrackParamTruthInitConfig>,
        public SpdlogMixin<TrackParamTruthInit_factory>  {

    public:
        TrackParamTruthInit_factory( std::vector<std::string> default_input_tags):
                JChainFactoryT<eicrecon::TrackParameters>(std::move(default_input_tags) ) {
        }

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:

        eicrecon::TrackParamTruthInit m_truth_track_seeding_algo;  /// Truth track seeding algorithm
    };

} // eicrecon

#endif //EICRECON_TrackParamTruthInit_factory_H
