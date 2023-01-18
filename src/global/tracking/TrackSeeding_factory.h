// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_TrackSeeding_factory_H
#define EICRECON_TrackSeeding_factory_H

#include <spdlog/spdlog.h>

#include <algorithms/tracking/TrackSeeding.h>
#include <algorithms/tracking/TrackerSourceLinkerResult.h>

#include <extensions/spdlog/SpdlogMixin.h>
#include <extensions/jana/JChainFactoryT.h>
#include <edm4eic/TrackParameters.h>


namespace eicrecon {

    class TrackSeeding_factory :
            public JChainFactoryT<edm4eic::TrackParameters, OrthogonalTrackSeedingConfig>,
            public SpdlogMixin<TrackSeeding_factory> {

    public:
        TrackSeeding_factory( std::vector<std::string> default_input_tags, OrthogonalTrackSeedingConfig cfg):
                JChainFactoryT<edm4eic::TrackParameters, OrthogonalTrackSeedingConfig>(std::move(default_input_tags), cfg ) {
        }

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:

        eicrecon::TrackSeeding m_seeding_algo;                      /// Proxy tracking algorithm

    };

} // eicrecon

#endif //EICRECON_TrackSeeding_factory_H
