// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_TRACKINGRESULT_FACTORY_H
#define EICRECON_TRACKINGRESULT_FACTORY_H

#include <algorithms/tracking/ParticlesFromTrackFitResult.h>
#include <algorithms/tracking/ParticlesFromTrackFit.h>
#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>

class TrackingResult_factory:
        public JChainFactoryT<ParticlesFromTrackFitResult>,
        public eicrecon::SpdlogMixin<TrackingResult_factory> {
public:
    explicit TrackingResult_factory(std::vector<std::string> default_input_tags):
    JChainFactoryT<ParticlesFromTrackFitResult>(std::move(default_input_tags) ) {
    }

    /** One time initialization **/
    void Init() override;

    /** On run change preparations **/
    void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

    /** Event by event processing **/
    void Process(const std::shared_ptr<const JEvent> &event) override;

private:

    Jug::Reco::ParticlesFromTrackFit m_particle_maker_algo;      /// Track source linker algorithm
};


#endif //EICRECON_TRACKINGRESULT_FACTORY_H
