

#include "IRT/CherenkovDetector.h"
#include "IRT/ReconstructionFactory.h"

#include "IrtInterface.h"

// -------------------------------------------------------------------------------------

namespace eicrecon {
  void IrtInterface::JsonParser( void )
  {
    // For less typing;
    auto reco = m_ReconstructionFactory;
    /*const*/ nlohmann::json *jptr = &m_config.m_json_config;

    // Uncomment if timing information accounting in chi^2 fails to work properly;
    //-reco->IgnoreTimingInChiSquare();
    if (jptr->find("UseTimingInChiSquare") != jptr->end() &&
	  !strcmp((*jptr)["UseTimingInChiSquare"].template get<std::string>().c_str(), "no"))
      reco->IgnoreTimingInChiSquare();
    
    //+reco->IgnorePoissonTermInChiSquare();
    if (jptr->find("UsePoissonTermInChiSquare") != jptr->end() &&
	  !strcmp((*jptr)["UsePoissonTermInChiSquare"].template get<std::string>().c_str(), "no"))
    reco->IgnorePoissonTermInChiSquare();

    // Outlier hit selection;
    //-reco->SetSingleHitCCDFcut(0.003); // default: 0.001
    if (jptr->find("SingleHitCCDFcut") != jptr->end())
      reco->SetSingleHitCCDFcut((*jptr)["SingleHitCCDFcut"].template get<double>());
    
    // Uncomment if prefer to remove hits which seem to be "shared" between >1 track;
    //-reco->RemoveAmbiguousHits();
    if (jptr->find("RemoveAmbiguousHits") != jptr->end() &&
	  !strcmp((*jptr)["RemoveAmbiguousHits"].template get<std::string>().c_str(), "yes"))
      reco->RemoveAmbiguousHits();
    
    // Should be close enough to the real one; this only affects the calibration stage;
    //+reco->SetDefaultSinglePhotonThetaResolution(0.002);
    if (jptr->find("DefaultSinglePhotonThetaResolution") != jptr->end())
      reco->SetDefaultSinglePhotonThetaResolution((*jptr)["DefaultSinglePhotonThetaResolution"].template get<double>());
	
    // Sensor active area will be pixellated NxN in digitization; '32': HRPPD-like "sensors";
    //+reco->SetSensorActiveAreaPixellation(32);
    if (jptr->find("SensorActiveAreaPixellation") != jptr->end())
      reco->SetSensorActiveAreaPixellation((*jptr)["SensorActiveAreaPixellation"].template get<int>());
    
    //-reco->SetSinglePhotonTimingResolution(0.030); // default: 0.050 (50ps);
    if (jptr->find("SinglePhotonTimingResolution") != jptr->end())
      reco->SetSinglePhotonTimingResolution((*jptr)["SinglePhotonTimingResolution"].template get<double>());

    // PID hypotheses to consider;
    reco->AddHypothesis("pi+");
    reco->AddHypothesis(321);
    
    // Comment out if want to cheat a bit (feed IRT with true photon direction vectors);
    //+reco->IgnoreMcTruthPhotonDirectionSeed();
    if (jptr->find("UseMcTruthPhotonDirectionSeed") != jptr->end() &&
	  !strcmp((*jptr)["UseMcTruthPhotonDirectionSeed"].template get<std::string>().c_str(), "no"))
      reco->IgnoreMcTruthPhotonDirectionSeed();

    // Require at least that many associated hits; populate 0-th bin of a PID match
    // histogram otherwise; default: 1; 
    //+reco->SetHitCountCutoff(5);
    if (jptr->find("MinHitCountCutoff") != jptr->end())
      reco->SetHitCountCutoff((*jptr)["MinHitCountCutoff"].template get<int>());

#if 1
    if (jptr->find("Radiators") != jptr->end()) {
      const auto &rconfig = (*jptr)["Radiators"];
      
      for(auto [name,radiator] : reco->GetMyRICH()->Radiators()) {
	// There should be an entry in JSON file; skip otherwise;
	if (rconfig.find(name.Data()) == rconfig.end()) continue;
	const auto &rrconfig = rconfig[name.Data()];
	
	if (rrconfig.find("imaging") != rrconfig.end() &&
	    !strcmp(rrconfig["imaging"].template get<std::string>().c_str(), "yes"))
	  radiator->UseInRingImaging();
	  
	if (rrconfig.find("evaluation-plots") != rrconfig.end() &&
	    !strcmp(rrconfig["evaluation-plots"].template get<std::string>().c_str(), "yes")) {
	  TString tag(tolower(name.Data()[0]));
	  radiator->InitializePlots(tag);

	  {
	    auto plots = radiator->Plots();
	    
	    if (rrconfig.find("refractive-index-range") != rrconfig.end())
	      plots->SetRefractiveIndexRange(rrconfig["refractive-index-range"][0].template get<double>(),
					     rrconfig["refractive-index-range"][1].template get<double>());
	    
	    if (rrconfig.find("photon-vertex-range") != rrconfig.end())
	      plots->SetPhotonVertexRange(rrconfig["photon-vertex-range"][0].template get<double>(),
					  rrconfig["photon-vertex-range"][1].template get<double>());
	    
	    if (rrconfig.find("cherenkov-angle-range") != rrconfig.end())
	      plots->SetCherenkovAngleRange(rrconfig["cherenkov-angle-range"][0].template get<double>(),
					    rrconfig["cherenkov-angle-range"][1].template get<double>());
	  }
	} //if
      } //for radiator
    }
#else
    auto *ra = reco->GetMyRICH()->GetRadiator("Aerogel");
    ra->UseInRingImaging()->InitializePlots("a");
    if (ra->Plots()) {
      // Initialize aerogel QA plots; 
      ra->Plots()->SetRefractiveIndexRange(1.015, 1.025);
      ra->Plots()->SetPhotonVertexRange(2500, 2650);
      ra->Plots()->SetCherenkovAngleRange(180, 200);
    } //if
    auto *rg = reco->GetMyRICH()->GetRadiator("GasVolume");
    rg->UseInRingImaging()->InitializePlots("g");
    if (rg->Plots()) {
      // Initialize gas radiator QA plots; 
      rg->Plots()->SetRefractiveIndexRange(1.00050, 1.00100);
      rg->Plots()->SetPhotonVertexRange(2400, 4000);
      rg->Plots()->SetCherenkovAngleRange(30, 50);
    } //if
#endif
    
    // Initialize combined PID QA plots;
    //+reco->InitializePlots();
    if (jptr->find("BuildCombinedEvaluationPlots") != jptr->end() &&
	  !strcmp((*jptr)["BuildCombinedEvaluationPlots"].template get<std::string>().c_str(), "yes"))
      reco->InitializePlots();
  } // IrtInterface::JsonParser()
} // namespace eicrecon

// -------------------------------------------------------------------------------------
