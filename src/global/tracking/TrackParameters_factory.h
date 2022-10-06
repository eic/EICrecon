// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_TRACKPARAMETERS_FACTORY_H
#define EICRECON_TRACKPARAMETERS_FACTORY_H

#include <edm4eic/TrackParameters.h>
#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>



namespace eicrecon {

    class TrackParameters_factory : public JChainFactoryT<edm4eic::TrackParameters>,
                                    public SpdlogMixin<TrackParameters_factory> {

    public:
        TrackParameters_factory(std::vector<std::string> default_input_tags):
        JChainFactoryT<edm4eic::TrackParameters>( std::move(default_input_tags) ) {
        }

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:

        std::vector<std::string> m_input_tags;              /// Tag for the input data
    };

} // eicrecon

#endif //EICRECON_TRACKPARAMETERS_FACTORY_H
