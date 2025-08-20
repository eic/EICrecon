

#include "IRT/CherenkovDetector.h"
#include "IRT/ReconstructionFactory.h"

#include "IrtInterface.h"

// -------------------------------------------------------------------------------------

namespace eicrecon {
  void IrtInterface::JsonParser( void )
  {
    // For less typing;
    auto reco = m_ReconstructionFactory;

    // Uncomment if timing information accounting in chi^2 fails to work properly;
    //reco->IgnoreTimingInChiSquare();
    reco->IgnorePoissonTermInChiSquare();

    // Outlier hit selection;
    //reco->SetSingleHitCCDFcut(0.003); // default: 0.001
    
    // Uncomment if prefer to remove hits which seem to be "shared" between >1 track;
    //reco->RemoveAmbiguousHits();
    
    // Should be close enough to the real one; this only affects the calibration stage;
    reco->SetDefaultSinglePhotonThetaResolution(0.002);
    // Sensor active area will be pixellated NxN in digitization; '32': HRPPD-like "sensors";
    reco->SetSensorActiveAreaPixellation(32);
    
    //reco->SetSinglePhotonTimingResolution(0.030); // default: 0.050 (50ps);

    // PID hypotheses to consider;
    reco->AddHypothesis("pi+");
    reco->AddHypothesis(321);
    
    // Comment out if want to cheat a bit (feed IRT with true photon direction vectors);
    reco->IgnoreMcTruthPhotonDirectionSeed();

    // Require at least that many associated hits; populate 0-th bin of a PID match
    // histogram otherwise; default: 1; 
    reco->SetHitCountCutoff(5);

    
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
    // Initialize combined PID QA plots;
    reco->InitializePlots();
  } // IrtInterface::JsonParser()
} // namespace eicrecon

// -------------------------------------------------------------------------------------
