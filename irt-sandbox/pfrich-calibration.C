//
//   root -l './pfrich-calibration.C("pfrich-events.root", "pfrich-calibration.json")'
//

void pfrich_calibration(const char* dfname, const char* jfname, unsigned stat = 0) {
  auto* reco = new ReconstructionFactory(dfname, dfname, "PFRICH");

  //
  // Factory configuration part; use minimal set of options;
  //
  reco->IgnorePoissonTermInChiSquare();
  // Should be close enough to the real one; this only affects the calibration stage;
  reco->SetDefaultSinglePhotonThetaResolution(0.005);
  // Sensor active area will be pixellated NxN in digitization; '32': HRPPD;
  reco->SetSensorActiveAreaPixellation(32);

  // One hypothesis suffices to run the reconstruction engine;
  reco->AddHypothesis("pi+");

  // Enable both radiators of interest for ring imaging;
  reco->GetMyRICH()->GetRadiator("Aerogel")->UseInRingImaging();

  // Perform pre-calibration; second argument: statistics to use (default: all events);
  reco->PerformCalibration(stat);
  // Export a JSON file with these calibrations;
  reco->GetMyRICH()->ExportJsonFormatCalibrations(jfname);
} // pfrich_calibration()
