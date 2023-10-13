// Eta coverage of detectors
// Shyam Kumar:INFN Bari, shyam.kumar@ba.infn.it; shyam055119@gmail.com

#include <TGraphErrors.h>
#include <TF1.h>
#include <TRandom.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TMath.h>
#define mpi 0.139  // 1.864 GeV/c^2

void Plot_eta()
{

   //==style of the plot====
   gStyle->SetPalette(1);
   gStyle->SetOptTitle(0);
   gStyle->SetTitleOffset(.85,"X");gStyle->SetTitleOffset(1.0,"Y");
   gStyle->SetTitleSize(.05,"X");gStyle->SetTitleSize(.05,"Y");
   gStyle->SetLabelSize(.04,"X");gStyle->SetLabelSize(.04,"Y");
   gStyle->SetHistLineWidth(2);
   gStyle->SetOptFit(1);
   gStyle->SetOptStat(0);


   TFile *f = TFile::Open("sim.edm4hep.root");
   TTree *tree = (TTree*)f->Get("events");

 // Timer Start
  TStopwatch timer;
  timer.Start();

   TCanvas * c1 = new TCanvas("c1","coutput",1400,1000);
   c1->SetMargin(0.10, 0.05 ,0.1,0.05);
   c1->SetGridx();

   TH1D *hits_vtx_si = new TH1D("hits_vtx_si","hits_vtx_si",200,-4.,4.);
   TH1D *hits_barrel_si = new TH1D("hits_barrel_si","hits_barrel_si",200,-4.,4.); // Barr1
   TH1D *hits_barrel_mpgd_in = new TH1D("hits_barrel_mpgd_in","hits_barrel_mpgd_in",200,-4.,4.); // BMT1
   TH1D *hits_barrel_tof = new TH1D("hits_barrel_tof","hits_barrel_tof",200,-4.,4.);
   TH1D *hits_disks_si = new TH1D("hits_disks_si","hits_disks_si",200,-4.,4.);
   TH1D *hits_endcap_tof = new TH1D("hits_endcap_tof","hits_endcap_tof",200,-4.,4.);
   TH1D *hits_barrel_mpgd_out = new TH1D("hits_barrel_mpgd_out","hits_barrel_mpgd_out",200,-4.,4.);
   TH1D *hits_fwd_mpgd = new TH1D("hits_fwd_mpgd","hits_fwd_mpgd",200,-4.,4.);
   TH1D *hits_bwd_mpgd = new TH1D("hits_bwd_mpgd","hits_bwd_mpgd",200,-4.,4.);
   TH1D *hits_b0tracker = new TH1D("hits_b0tracker","hits_b0tracker",200,-4.,4.);

   hits_vtx_si->GetXaxis()->SetTitle("#eta");
   hits_vtx_si->GetYaxis()->SetTitle("Entries (a.u.)");
   hits_vtx_si->GetXaxis()->CenterTitle();
   hits_vtx_si->GetYaxis()->CenterTitle();

   TString vtx_hits_eta = "-TMath::Log(TMath::Tan((TMath::ATan2(sqrt(VertexBarrelHits.position.x*VertexBarrelHits.position.x+VertexBarrelHits.position.y*VertexBarrelHits.position.y),VertexBarrelHits.position.z))/2))>>hits_vtx_si";
   TString Barrel_hits_eta = "-TMath::Log(TMath::Tan((TMath::ATan2(sqrt(SiBarrelHits.position.x*SiBarrelHits.position.x+SiBarrelHits.position.y*SiBarrelHits.position.y),SiBarrelHits.position.z))/2))>>hits_barrel_si";
   TString BMM_hits_eta = "-TMath::Log(TMath::Tan((TMath::ATan2(sqrt(MPGDBarrelHits.position.x*MPGDBarrelHits.position.x+MPGDBarrelHits.position.y*MPGDBarrelHits.position.y),MPGDBarrelHits.position.z))/2))>>hits_barrel_mpgd_in";
   TString ToF_hits_eta = "-TMath::Log(TMath::Tan((TMath::ATan2(sqrt(TOFBarrelHits.position.x*TOFBarrelHits.position.x+TOFBarrelHits.position.y*TOFBarrelHits.position.y),TOFBarrelHits.position.z))/2))>>hits_barrel_tof";
   TString ETracker_hits_eta = "-TMath::Log(TMath::Tan((TMath::ATan2(sqrt(TrackerEndcapHits.position.x*TrackerEndcapHits.position.x+TrackerEndcapHits.position.y*TrackerEndcapHits.position.y),TrackerEndcapHits.position.z))/2))>>hits_disks_si";
   TString ETOF_hits_eta = "-TMath::Log(TMath::Tan((TMath::ATan2(sqrt(TOFEndcapHits.position.x*TOFEndcapHits.position.x+TOFEndcapHits.position.y*TOFEndcapHits.position.y),TOFEndcapHits.position.z))/2))>>hits_endcap_tof";
   TString OutBMM_hits_eta = "-TMath::Log(TMath::Tan((TMath::ATan2(sqrt(OuterMPGDBarrelHits.position.x*OuterMPGDBarrelHits.position.x+OuterMPGDBarrelHits.position.y*OuterMPGDBarrelHits.position.y),OuterMPGDBarrelHits.position.z))/2))>>hits_barrel_mpgd_out";
   TString FwdMM_hits_eta = "-TMath::Log(TMath::Tan((TMath::ATan2(sqrt(ForwardMPGDEndcapHits.position.x*ForwardMPGDEndcapHits.position.x+ForwardMPGDEndcapHits.position.y*ForwardMPGDEndcapHits.position.y),ForwardMPGDEndcapHits.position.z))/2))>>hits_fwd_mpgd";
   TString BwdMM_hits_eta = "-TMath::Log(TMath::Tan((TMath::ATan2(sqrt(BackwardMPGDEndcapHits.position.x*BackwardMPGDEndcapHits.position.x+BackwardMPGDEndcapHits.position.y*BackwardMPGDEndcapHits.position.y),BackwardMPGDEndcapHits.position.z))/2))>>hits_bwd_mpgd";
   TString B0_hits_eta = "-TMath::Log(TMath::Tan((TMath::ATan2(sqrt(B0TrackerHits.position.x*B0TrackerHits.position.x+B0TrackerHits.position.y*B0TrackerHits.position.y),B0TrackerHits.position.z))/2))>>hits_b0tracker";

   tree->Draw(vtx_hits_eta.Data(),"VertexBarrelHits.position.y>0","goff"); // Vtx layers
   tree->Draw(Barrel_hits_eta.Data(),"SiBarrelHits.position.y>0 ","goff"); // Barrel Si layers
   tree->Draw(BMM_hits_eta.Data(),"MPGDBarrelHits.position.y>0 ","goff"); // MPGD Barrel
   tree->Draw(ToF_hits_eta.Data(),"TOFBarrelHits.position.y>0","goff"); // TOF Hits
   tree->Draw(ETracker_hits_eta.Data(),"TrackerEndcapHits.position.y>0","goff"); // Tracker Endcap
   tree->Draw(ETOF_hits_eta.Data(),"TOFEndcapHits.position.y>0","goff"); // TOF Endcap
   tree->Draw(OutBMM_hits_eta.Data(),"OuterMPGDBarrelHits.position.y>0","goff"); // Outer Barrel MPGD
   tree->Draw(FwdMM_hits_eta.Data(),"ForwardMPGDEndcapHits.position.y>0","goff"); // Forward MPGD
   tree->Draw(BwdMM_hits_eta.Data(),"BackwardMPGDEndcapHits.position.y>0","goff"); // Forward MPGD
   tree->Draw(B0_hits_eta.Data(),"B0TrackerHits.position.y>0","goff"); // B0 Tracker
  
   c1->cd();
   c1->SetLogy();
   hits_vtx_si->Scale(1./hits_vtx_si->Integral());
   hits_barrel_si->Scale(1./hits_barrel_si->Integral());
   hits_barrel_mpgd_in->Scale(1./hits_barrel_mpgd_in->Integral());
   hits_barrel_tof->Scale(1./hits_barrel_tof->Integral());
   hits_disks_si->Scale(1./hits_disks_si->Integral());
   hits_endcap_tof->Scale(1./hits_endcap_tof->Integral());
   hits_barrel_mpgd_out->Scale(1./hits_barrel_mpgd_out->Integral());
   hits_fwd_mpgd->Scale(1./hits_fwd_mpgd->Integral());
   hits_bwd_mpgd->Scale(1./hits_bwd_mpgd->Integral());
   hits_b0tracker->Scale(1./hits_b0tracker->Integral());
   hits_vtx_si->SetLineColor(kBlue);
   hits_barrel_si->SetLineColor(kMagenta);
   hits_barrel_mpgd_in->SetLineColor(kRed);
   hits_barrel_tof->SetLineColor(kBlack);
   hits_disks_si->SetLineColor(kGreen);
   hits_endcap_tof->SetLineColor(kCyan);
   hits_barrel_mpgd_out->SetLineColor(kOrange);
   hits_fwd_mpgd->SetLineColor(kAzure);
   hits_bwd_mpgd->SetLineColor(kTeal);
   hits_b0tracker->SetLineColor(kViolet);


   hits_vtx_si->SetMaximum(0.2);
   hits_vtx_si->GetXaxis()->SetRangeUser(-4.0,4.0);
   hits_vtx_si->Draw("hist");
   hits_barrel_si->Draw("hist-same");
   hits_barrel_mpgd_in->Draw("hist-same");
   hits_barrel_tof->Draw("hist-same");
   hits_disks_si->Draw("hist-same");
   hits_endcap_tof->Draw("hist-same");
   hits_barrel_mpgd_out->Draw("hist-same");
   hits_fwd_mpgd->Draw("hist-same");
   hits_bwd_mpgd->Draw("hist-same");
   hits_b0tracker->Draw("hist-same");

  TLegend *l1=  new TLegend(0.11,0.75,0.70,0.94);
  l1->SetNColumns(2);
  l1->SetTextSize(0.025);
  l1->SetBorderSize(0);
  l1->AddEntry(hits_vtx_si,"VertexBarrelHits");
  l1->AddEntry(hits_barrel_si,"SiBarrelHits");
  l1->AddEntry(hits_barrel_mpgd_in,"MPGDBarrelHits");
  l1->AddEntry(hits_barrel_tof,"TOFBarrelHits");
  l1->AddEntry(hits_disks_si,"TrackerEndcapHits");
  l1->AddEntry(hits_endcap_tof,"TOFEndcapHits");
  l1->AddEntry(hits_barrel_mpgd_out,"OuterMPGDBarrelHits");
  l1->AddEntry(hits_fwd_mpgd,"ForwardMPGDEndcapHits");
  l1->AddEntry(hits_bwd_mpgd,"BackwardMPGDEndcapHits");
  l1->AddEntry(hits_b0tracker,"B0TrackerHits");
  l1->Draw("same");
  c1->cd();
  l1->Draw();
  c1->SaveAs("eta_DD4HEP.png");
  
 // Timer Stop     
  timer.Stop();
  Double_t realtime = timer.RealTime();
  Double_t cputime = timer.CpuTime();
  printf("RealTime=%f seconds, CpuTime=%f seconds\n",realtime,cputime);  

}
