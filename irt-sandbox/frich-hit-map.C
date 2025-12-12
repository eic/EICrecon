//
//   root -l './frich-hit-map.C("frich-events.root", "frich-optics.root")'
//
//    or
//
//   root -l './frich-hit-map.C("frich-events.root")'
//

void frich_hit_map(const char* dfname, const char* cfname = 0) {
  auto fcfg = new TFile(cfname ? cfname : dfname);
  auto geometry =
      dynamic_cast<CherenkovDetectorCollection*>(fcfg->Get("CherenkovDetectorCollection"));
  auto fdata = new TFile(dfname);
  TTree* t   = dynamic_cast<TTree*>(fdata->Get("t"));
  auto event = new CherenkovEvent();
  t->SetBranchAddress("e", &event);

  int nEvents = t->GetEntries();
  printf("%d event(s) total\n", nEvents);

  //+auto hxy = new TH2D("hxy", "", 1200, -1200., 1200., 1200, -1200.0, 1200.);
  auto hxy = new TH2D("hxy", "", 500, -500., 500., 500, -500.0, 500.);

  for (unsigned ev = 0; ev < nEvents; ev++) {
    t->GetEntry(ev);

    for (auto particle : event->ChargedParticles()) {

      for (auto rhistory : particle->GetRadiatorHistory()) {
        auto history = particle->GetHistory(rhistory);

        for (auto photon : history->Photons()) {
          if (!photon->WasDetected())
            continue;

          TVector3 phx = photon->GetDetectionPosition();
          hxy->Fill(phx.X(), phx.Y());
        } //for photon
      } //for rhistory
    } //for particle
  } //for ev

  gStyle->SetOptStat(0);
  auto cv = new TCanvas("cv", "", 1000, 1000);
  hxy->GetXaxis()->SetTitle("Sensor plane X, [mm]");
  hxy->GetYaxis()->SetTitle("Sensor plane Y, [mm]");
  hxy->GetXaxis()->SetTitleOffset(1.20);
  hxy->GetYaxis()->SetTitleOffset(1.40);
  hxy->Draw("COL");
} // frich_hit_map()
