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
#include "factories/postburn/PostBurnRecoParticles_factory.h"



extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    using namespace eicrecon;


	PostBurnConfig postburn_config;
	PostBurnConfig pseudoPostBurn_config;


	//Full correction for MCParticles --> MCParticlesPostBurn
	postburn_config.pidAssumePionMass = false;
	postburn_config.crossingAngle    = 0.025 * dd4hep::rad;
	postburn_config.pidPurity        = 0.51;
	postburn_config.correctBeamFX    = true;
	postburn_config.pidUseMCTruth    = true;

	//Pseudo post burn for ReconstructedChargedParticles --> ReconstructedChargedParticlesPsuedoPostBurn
	pseudoPostBurn_config.pidAssumePionMass = true;
	pseudoPostBurn_config.crossingAngle    = 0.025 * dd4hep::rad;
	pseudoPostBurn_config.pidPurity        = 0.51;
	pseudoPostBurn_config.correctBeamFX    = false;
	pseudoPostBurn_config.pidUseMCTruth    = false;


	//Need to read-in MCParticles and ReconstructedChargedParticles

	app->Add(new JOmniFactoryGeneratorT<PostBurnMCParticles_factory>(
	        "MCParticlesPostBurn",
	        {
	          "MCParticles",
	          "ReconstructedChargedParticles",
	          "ReconstructedChargedParticleAssociations"
	        },
	        {
	          "MCParticlesPostBurn"
	        },
			postburn_config,
			app
	    ));
			
	app->Add(new JOmniFactoryGeneratorT<PostBurnRecoParticles_factory>(
	        "ReconstructedChargedParticlesPsuedoPostBurn",
	        {
	          "MCParticles",
	          "ReconstructedChargedParticles",
	          "ReconstructedChargedParticleAssociations"
	        },
	        {
	          "ReconstructedChargedParticlesPsuedoPostBurn"
	        },
			pseudoPostBurn_config,
	        app
	    ));
			
}
}
