// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <algorithms/tracking/ParticlesFromTrackFitResult.h>
#include <algorithms/tracking/ParticlesFromTrackFit.h>
#include <extensions/jana/JChainMultifactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>

class TrackingResult_factory:
        public JChainMultifactoryT<NoConfig>,
        public eicrecon::SpdlogMixin<TrackingResult_factory> {
public:
    explicit TrackingResult_factory(std::string tag,
                                    const std::vector<std::string>& input_tags,
                                    const std::vector<std::string>& output_tags):
    JChainMultifactoryT(std::move(tag), input_tags, output_tags) {

        DeclarePodioOutput<edm4eic::ReconstructedParticle>(GetOutputTags()[0]);
        DeclarePodioOutput<edm4eic::TrackParameters>(GetOutputTags()[1]);
    }


    /** One time initialization **/
    void Init() override;

    /** On run change preparations **/
    void BeginRun(const std::shared_ptr<const JEvent> &event) override;

    /** Event by event processing **/
    void Process(const std::shared_ptr<const JEvent> &event) override;

private:

    eicrecon::Reco::ParticlesFromTrackFit m_particle_maker_algo;      /// Track source linker algorithm
};
