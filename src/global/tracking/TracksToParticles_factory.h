// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <algorithms/tracking/TracksToParticles.h>
#include <extensions/jana/JChainMultifactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>

class TracksToParticles_factory:
        public JChainMultifactoryT<NoConfig>,
        public eicrecon::SpdlogMixin<TracksToParticles_factory> {
public:
    explicit TracksToParticles_factory(std::string tag,
                                    const std::vector<std::string>& input_tags,
                                    const std::vector<std::string>& output_tags):
    JChainMultifactoryT(std::move(tag), input_tags, output_tags) {

        DeclarePodioOutput<edm4eic::ReconstructedParticle>(GetOutputTags()[0]);
    }


    /** One time initialization **/
    void Init() override;

    /** On run change preparations **/
    void BeginRun(const std::shared_ptr<const JEvent> &event) override;

    /** Event by event processing **/
    void Process(const std::shared_ptr<const JEvent> &event) override;

private:

    eicrecon::Reco::TracksToParticles m_particle_maker_algo;  
};
