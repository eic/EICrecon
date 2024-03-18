// Copyright 2024, Alex Jentsch, Jihee Kim, Brian Page
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <vector>

//#include "algorithms/fardetectors/MatrixTransferStaticConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/postburn/PostBurnMCParticles_factory.h"
#include "factories/postburn/PostBurnRecoParticles_factory.h"



extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    using namespace eicrecon;


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
			{
				.crossingAngle = -0.025 * dd4hep::rad;
				.correctBeamFX = true;
				.pidPurity = 0.51; //dummy for now
				.pidAssumePionMass = false;
				
			}
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
			{
				.crossingAngle = -0.025 * dd4hep::rad;
				.correctBeamFX = false;
				.pidPurity = 0.51; //dummy for now
				.pidAssumePionMass = true;
		
			}
	        app
	    ));
	

   

    app->Add(new JOmniFactoryGeneratorT<MatrixTransferStatic_factory>("ForwardRomanPotRecParticles",{"MCParticles","ForwardRomanPotRecHits"},{"ForwardRomanPotRecParticles"},recon_cfg,app));

}
}
