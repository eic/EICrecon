// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Alex Jentsch, Jihee Kim, Brian Page
//


#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <vector>

#include "algorithms/reco/PostBurnConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/reco/PostBurnMCParticles_factory.h"



extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    using namespace eicrecon;


    PostBurnConfig postburn_config;


    //Full correction for MCParticles --> MCParticlesHeadOnFrame
    postburn_config.m_pid_assume_pion_mass = false;
    postburn_config.m_crossing_angle    = -0.025 * dd4hep::rad;
    postburn_config.m_pid_purity        = 0.51; //dummy value for MC truth information
    postburn_config.m_correct_beam_FX    = true;
    postburn_config.m_pid_use_MC_truth    = true;


    //Need to read-in MCParticles

    app->Add(new JOmniFactoryGeneratorT<PostBurnMCParticles_factory>(
            "MCParticlesHeadOnFrameNoBeamFX",
            {
                "MCParticles"
            },
            {
                "MCParticlesHeadOnFrameNoBeamFX"
            },
            postburn_config,
            app
            ));


}
}
