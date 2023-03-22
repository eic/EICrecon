// Created by Shyam Kumar; INFN Bari, shyam.kumar@ba.infn.it; shyam055119@gmail.com
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <edm4eic/Trajectory.h>
#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>

namespace eicrecon {

    class Trajectory_factory : public JChainFactoryT<edm4eic::Trajectory>,
                                    public SpdlogMixin<Trajectory_factory> {

    public:
        Trajectory_factory(std::vector<std::string> default_input_tags):
        JChainFactoryT<edm4eic::Trajectory>( std::move(default_input_tags) ) {
        }

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:

        std::vector<std::string> m_input_tags; 
    };

} // eicrecon
