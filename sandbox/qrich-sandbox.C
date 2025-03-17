//
//   export ROOT_LIBRARY_PATH=/DATA00/ayk/ePIC/prefix/lib:${ROOT_LIBRARY_PATH}
//
//   root -l './qrich-sandbox.C("qrich-events.root", "qrich-optics.root")'
//

void qrich_sandbox(const char *dfname, const char *cfname = 0)
{
  auto fcfg  = new TFile(cfname ? cfname : dfname);
  auto geometry = dynamic_cast<CherenkovDetectorCollection*>(fcfg->Get("CherenkovDetectorCollection"));
  auto fdata = new TFile(dfname);
  TTree *t = dynamic_cast<TTree*>(fdata->Get("t")); 
  auto event = new CherenkovEvent();
  t->SetBranchAddress("e", &event);

  auto qrich = geometry->GetDetector("QRICH");

  int nEvents = t->GetEntries();
  printf("%d events total\n", nEvents);

  auto hxy  = new TH2D("hxy",  "", 650, -650., 650., 650, -650.0, 650.);
  auto npe  = new TH1D("npe",  "", 100, 0, 100);
  auto zvtx = new TH1D("zvtx", "", 50, 1300., 1350.);

  for(unsigned ev=0; ev<nEvents; ev++) {
    t->GetEntry(ev);

    printf("%ld\n", event->ChargedParticles().size());
    for(auto particle: event->ChargedParticles()) {

      printf("  %ld\n", particle->GetRadiatorHistory().size());
      for(auto rhistory: particle->GetRadiatorHistory()) {
	auto history  = particle->GetHistory (rhistory);
	auto radiator = particle->GetRadiator(rhistory);
	bool aerogel = radiator == qrich->GetRadiator("Aerogel");
	
	printf("    %ld\n", history->Photons().size());
	unsigned detected = 0;
	for(auto photon: history->Photons()) {
	  printf("  det: %d, cal: %d\n", photon->WasDetected(), photon->IsUsefulForCalibration());
	  if (!photon->WasDetected()/* || photon->IsUsefulForCalibration()*/) continue;

	  detected++;
	  
	  TVector3 phx = photon->GetDetectionPosition();
	  hxy->Fill(phx.X(), phx.Y());
	  printf("   -> detected %7.2f %7.2f %8.2f!\n", phx.X(), phx.Y(), phx.Z());
	  if (aerogel) zvtx->Fill(photon->GetVertexPosition().Z());
	} //for photon

	if (aerogel) npe->Fill(detected);//history->Photons().size());
      } //for rhistory
    } //for particle
    //#endif
  } //for ev

#if 1//_TODAY_
  //gStyle->SetOptStat(0);
  auto cv = new TCanvas("cv", "", 1500, 500);
  cv->Divide(3,1);
  cv->cd(1);
  hxy->GetXaxis()->SetTitle("Sensor plane X, [mm]");
  hxy->GetYaxis()->SetTitle("Sensor plane Y, [mm]");
  hxy->GetXaxis()->SetTitleOffset(1.20);
  hxy->GetYaxis()->SetTitleOffset(1.40);
  hxy->SetMaximum(10);
  hxy->Draw("COLZ");
  
  cv->cd(2);
  npe->Draw();
  
  cv->cd(3);
  zvtx->Draw();
#endif
} // hit_map_qrich()
