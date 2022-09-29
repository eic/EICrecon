// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_TRACKINGRESULT_FACTORY_H
#define EICRECON_TRACKINGRESULT_FACTORY_H

#include <algorithms/tracking/ParticlesFromTrackFitResult.h>
#include <algorithms/tracking/ParticlesFromTrackFit.h>
#include "extensions/jana/JChainFactoryT.h"
#include <spdlog/logger.h>

class TrackingResult_factory: public JChainFactoryT<ParticlesFromTrackFitResult> {
public:
    TrackingResult_factory(std::vector<std::string> default_input_tags ):
    JChainFactoryT<ParticlesFromTrackFitResult>(std::move(default_input_tags) ) {
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


    Jug::Reco::ParticlesFromTrackFit m_particle_maker_algo;      /// Track source linker algorithm

    std::vector<std::string> m_input_tags;              /// Tags of factories that provide input data

};


#endif //EICRECON_TRACKINGRESULT_FACTORY_H
