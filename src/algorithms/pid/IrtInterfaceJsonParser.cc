
#include "IRT2/CherenkovDetector.h"
#include "IRT2/ReconstructionFactory.h"

using namespace IRT2;

#include "IrtInterface.h"

using json = nlohmann::json;

// -------------------------------------------------------------------------------------

namespace eicrecon {
  void IrtInterface::JsonParser( void )
  {
    //printf("@R@ IrtInterface::JsonParser()\n");
    
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

    // PID hypotheses to consider; FIXME: make configurable;
    //reco->AddHypothesis("pi+");
    //reco->AddHypothesis(321);"pi+"
    //"IdentifiedParticles": ["pi+", "K+"], 
    if (jptr->find("IdentifiedParticles") != jptr->end()) {
      const auto &pconfig = (*jptr)["IdentifiedParticles"];
      
      for(auto &pdg: pconfig)
	reco->AddHypothesis(pdg.template get<std::string>().c_str());
    } //if
    
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
    
    // FIXME: this field should be mandatory (add a try-catch or such);
    if (jptr->find("Calibration") != jptr->end()) {
      std::ifstream fcalib((*jptr)["Calibration"].template get<std::string>().c_str());
      if (fcalib.is_open()) {
	auto jcalib = json::parse(fcalib);
	
	if (jcalib.find("Radiators") != jcalib.end()) {
	  const auto &rconfig = jcalib["Radiators"];
	  
	  for(auto [name,radiator] : reco->GetMyRICH()->Radiators()) {
	    // There should be an entry in JSON file; skip otherwise;
	    if (rconfig.find(name.Data()) == rconfig.end()) continue;
	    const auto &rrconfig = rconfig[name.Data()];

	    // Prefer to initialize in a separate loop; clear() is not really needed (?);
	    radiator->m_Calibrations.clear();
	    for(unsigned iq=0; iq<_THETA_BIN_COUNT_; iq++)
	      radiator->m_Calibrations.push_back(CherenkovRadiatorCalibration());
      
	    if (rrconfig.find("theta-bins") != rrconfig.end()) {
	      const auto &tconfig = rrconfig["theta-bins"];
	      for(unsigned iq=0; iq<_THETA_BIN_COUNT_; iq++) {
		TString bin; bin.Form("%02d", iq);
		
		if (tconfig.find(bin.Data()) != tconfig.end()) {
		  const auto &tarray = tconfig[bin.Data()];

		  auto *calib = &radiator->m_Calibrations[iq];

		  int rnum = (int)tarray.size() - 4;
		  //for(auto ip=0; ip<tarray.size(); ip++)
		  //printf("@R@ %s\n", tarray[ip].template get<std::string>().c_str());

		  if (rnum == reco->GetMyRICH()->Radiators().size()) {
		    calib->m_Stat        = atoi(tarray[0].template get<std::string>().c_str());
		    calib->m_AverageZvtx = atof(tarray[1].template get<std::string>().c_str());
		    // Convert back to [rad];
		    calib->m_Coffset     = atof(tarray[2].template get<std::string>().c_str())/1000.;
		    calib->m_Csigma      = atof(tarray[3].template get<std::string>().c_str())/1000.;
		    for(unsigned ir=0; ir<rnum; ir++)
		      calib->m_AverageRefractiveIndices.push_back(atof(tarray[4+ir].template get<std::string>().c_str()));
		  } //if
		} //if
	      } //for iq
	    } //if
	  } //for radiator
	} //if
	  
	fcalib.close();
      } //if
    } //if
    
    if (jptr->find("Radiators") != jptr->end()) {
      const auto &rconfig = (*jptr)["Radiators"];
      
      for(auto [name,radiator] : reco->GetMyRICH()->Radiators()) {
	// There should be an entry in JSON file; skip otherwise;
	if (rconfig.find(name.Data()) == rconfig.end()) continue;
	const auto &rrconfig = rconfig[name.Data()];
	
	if (rrconfig.find("imaging") != rrconfig.end() &&
	    !strcmp(rrconfig["imaging"].template get<std::string>().c_str(), "yes"))
	  radiator->UseInRingImaging();
	  
	if (rrconfig.find("evaluation-plots") != rrconfig.end()) {
	  const auto &tag = rrconfig["evaluation-plots"];
	  
	  if (!strcmp(tag.template get<std::string>().c_str(), "store"))
	    radiator->InitializePlots(TString(name.Data()[0]).Data());
	  else
	    if (!strcmp(tag.template get<std::string>().c_str(), "display")) {
	      radiator->InitializePlots(TString(name.Data()[0]).Data());
	      radiator->m_OutputPlotVisualizationEnabled = true;
	    } //if
	  	  
	  if (rrconfig.find("evaluation-plots-geometry") != rrconfig.end()) {
	    const auto &gconfig = rrconfig["evaluation-plots-geometry"];
	    
	    if (gconfig.size() == 4) {
	      radiator->m_wtopx = gconfig[0].template get<int>();
	      radiator->m_wtopy = gconfig[1].template get<int>();
	      radiator->m_wx    = gconfig[2].template get<int>();
	      radiator->m_wy    = gconfig[3].template get<int>();
	    } //if
	  } //if

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
    
    // Initialize combined PID QA plots;
    //+reco->InitializePlots();
    if (jptr->find("CombinedEvaluationPlots") != jptr->end()) {
      const auto &tag = (*jptr)["CombinedEvaluationPlots"];

      if (!strcmp(tag.template get<std::string>().c_str(), "store"))
	reco->InitializePlots();
      else
	if (!strcmp(tag.template get<std::string>().c_str(), "display")) {
	  reco->InitializePlots();
	  m_CombinedPlotVisualizationEnabled = true;
	} //if
    } //if
    if (jptr->find("CombinedEvaluationPlotsGeometry") != jptr->end()) {
      const auto &gconfig = (*jptr)["CombinedEvaluationPlotsGeometry"];

      if (gconfig.size() == 4) {
	m_wtopx = gconfig[0].template get<int>();
	m_wtopy = gconfig[1].template get<int>();
	m_wx    = gconfig[2].template get<int>();
	m_wy    = gconfig[3].template get<int>();
      } //if
    } //if
  } // IrtInterface::JsonParser()
} // namespace eicrecon

// -------------------------------------------------------------------------------------
