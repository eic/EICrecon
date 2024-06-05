// Copyright 2024, Alex Jentsch, Jihee Kim, Brian Page
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <vector>

#include "algorithms/postburn/PostBurnConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/postburn/PostBurnMCParticles_factory.h"



extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    using namespace eicrecon;


	PostBurnConfig postburn_config;


	//Full correction for MCParticles --> MCParticlesHeadOnFrame
	postburn_config.pidAssumePionMass = false;
	postburn_config.crossingAngle    = -0.025 * dd4hep::rad;
	postburn_config.pidPurity        = 0.51; //dummy value for MC truth information
	postburn_config.correctBeamFX    = true;
	postburn_config.pidUseMCTruth    = true;


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
