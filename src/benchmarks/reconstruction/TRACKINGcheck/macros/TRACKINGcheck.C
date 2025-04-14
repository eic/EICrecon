

void EEMCcheck(void) {
  auto fil = new TFile("eicrecon.root");

  auto EcalEndcapNhits_hits_per_event = (TH1I*)fil->Get("EEMC/EcalEndcapNhits_hits_per_event");
  auto EcalEndcapNhits_occupancy      = (TH2I*)fil->Get("EEMC/EcalEndcapNhits_occupancy");
  auto EcalEndcapNhits_hit_energy     = (TH1D*)fil->Get("EEMC/EcalEndcapNhits_hit_energy");

  auto EcalEndcapNRawhits_hits_per_event =
      (TH1I*)fil->Get("EEMC/EcalEndcapNRawhits_hits_per_event");
  auto EcalEndcapNRawhits_amplitude = (TH1D*)fil->Get("EEMC/EcalEndcapNRawhits_amplitude");
  auto EcalEndcapNRawhits_timestamp = (TH1I*)fil->Get("EEMC/EcalEndcapNRawhits_timestamp");

  auto EcalEndcapNRechits_hits_per_event =
      (TH1I*)fil->Get("EEMC/EcalEndcapNRechits_hits_per_event");
  auto EcalEndcapNRecHits_hit_energy = (TH1D*)fil->Get("EEMC/EcalEndcapNRecHits_hit_energy");
  auto EcalEndcapNRecHits_xy         = (TH2D*)fil->Get("EEMC/EcalEndcapNRecHits_xy");
  auto EcalEndcapNRecHits_z          = (TH1D*)fil->Get("EEMC/EcalEndcapNRecHits_z");
  auto EcalEndcapNRecHits_time       = (TH1D*)fil->Get("EEMC/EcalEndcapNRecHits_time");

  auto EcalEndcapNIslandProtoClusters_clusters_per_event =
      (TH1I*)fil->Get("EEMC/EcalEndcapNIslandProtoClusters_clusters_per_event");
  auto EcalEndcapNIslandProtoClusters_hits_per_cluster =
      (TH1I*)fil->Get("EEMC/EcalEndcapNIslandProtoClusters_hits_per_cluster");

  auto c1 = new TCanvas("c1", "", 1700, 1000);
  c1->Divide(4, 4);

  //---------------- EcalEndcapNhits
  c1->cd(1);
  gPad->SetLogy();
  EcalEndcapNhits_hits_per_event->Draw();

  c1->cd(2);
  EcalEndcapNhits_occupancy->Draw();

  c1->cd(3);
  gPad->SetLogy();
  EcalEndcapNhits_hit_energy->Draw();

  //---------------- EcalEndcapNRawhits
  c1->cd(5);
  gPad->SetLogy();
  EcalEndcapNRawhits_hits_per_event->Draw();

  c1->cd(6);
  gPad->SetLogy();
  EcalEndcapNRawhits_amplitude->Draw();

  c1->cd(7);
  EcalEndcapNRawhits_timestamp->Draw();

  //---------------- EcalEndcapNRecHits
  c1->cd(9);
  gPad->SetLogy();
  EcalEndcapNRechits_hits_per_event->Draw();

  c1->cd(10);
  EcalEndcapNRecHits_xy->Draw();

  c1->cd(11);
  EcalEndcapNRecHits_z->Draw();

  c1->cd(12);
  gPad->SetLogy();
  EcalEndcapNRecHits_time->Draw();

  c1->cd(8);
  gPad->SetLogy();
  EcalEndcapNRecHits_hit_energy->Draw();

  //---------------- EcalEndcapNIslandProtoClusters
  c1->cd(13);
  gPad->SetLogy();
  EcalEndcapNIslandProtoClusters_clusters_per_event->Draw();

  c1->cd(14);
  gPad->SetLogy();
  EcalEndcapNIslandProtoClusters_hits_per_cluster->Draw();

  c1->SaveAs("EEMCcheck.pdf");
  c1->SaveAs("EEMCcheck.png");
}
