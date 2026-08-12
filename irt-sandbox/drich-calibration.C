//
//   root -l './drich-calibration.C("drich-events.root", "drich-calibration.json")'
//

void drich_calibration(const char* dfname, const char* jfname, unsigned stat = 0) {
  auto* reco = new ReconstructionFactory(dfname, dfname, "DRICH");

  //
  // Factory configuration part; use minimal set of options;
  //
  reco->IgnorePoissonTermInChiSquare();
  // Should be close enough to the real one; this only affects the calibration stage;
  reco->SetDefaultSinglePhotonThetaResolution(0.002);
  // Sensor active area will be pixellated NxN in digitization; '8': SiPM panels;
  reco->SetSensorActiveAreaPixellation(8);

  // One hypothesis suffices to run the reconstruction engine;
  reco->AddHypothesis("pi+");

  // Enable both radiators of interest for ring imaging;
  reco->GetMyRICH()->GetRadiator("Aerogel")->UseInRingImaging();
  reco->GetMyRICH()->GetRadiator("GasVolume")->UseInRingImaging();

  // Perform pre-calibration; second argument: statistics to use (default: all events);
  reco->PerformCalibration(stat);
  // Export a JSON file with these calibrations;
  reco->GetMyRICH()->ExportJsonFormatCalibrations(jfname);
} // drich_calibration()
