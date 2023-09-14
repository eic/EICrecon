// Hits distribuition of detectors
// Shyam Kumar:INFN Bari, shyam.kumar@ba.infn.it; shyam055119@gmail.com

#include "TGraphErrors.h"
#include "TF1.h"
#include "TRandom.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TMath.h"

void draw_hits()
{

//==========Style of the plot============
   gStyle->SetPalette(1);
   gStyle->SetOptTitle(1);
   gStyle->SetTitleOffset(.85,"X");gStyle->SetTitleOffset(.85,"Y");
   gStyle->SetTitleSize(.04,"X");gStyle->SetTitleSize(.04,"Y");
   gStyle->SetLabelSize(.04,"X");gStyle->SetLabelSize(.04,"Y");
   gStyle->SetHistLineWidth(2);
   gStyle->SetOptFit(1);
   gStyle->SetOptStat(0);

//=======Reading the root file DD4HEP===========
 TFile *f = TFile::Open("sim.edm4hep.root");
 TTree *sim = (TTree*)f->Get("events");

 // Timer Start
  TStopwatch timer;
  timer.Start();

 // X-Y Hits
  Int_t nbins = 320;
  Double_t x= 100., y = 100.;
  TH2D *hitsxy_vtx_si = new TH2D("hitsxy_vtx_si","hitsxy_vtx_si",nbins,-x,x,nbins,-y,y);
  TH2D *hitsxy_barrel_si = new TH2D("hitsxy_barrel_si","hitsxy_barrel_si",nbins,-x,x,nbins,-y,y);
  TH2D *hitsxy_barrel_mm_in = new TH2D("hitsxy_barrel_mm_in","hitsxy_barrel_mm_in",nbins,-x,x,nbins,-y,y);
  TH2D *hitsxy_barrel_tof = new TH2D("hitsxy_barrel_tof","hitsxy_barrel_tof",nbins,-x,x,nbins,-y,y);
  TH2D *hitsxy_barrel_mm_out = new TH2D("hitsxy_barrel_mm_out","hitsxy_barrel_mm_out",nbins,-x,x,nbins,-y,y);

  TString si_vtx_hitsXY ="VertexBarrelHits.position.y*0.1:VertexBarrelHits.position.x*0.1>>hitsxy_vtx_si";
  TString si_barrel_hitsXY ="SiBarrelHits.position.y*0.1:SiBarrelHits.position.x*0.1>>hitsxy_barrel_si";
  TString barrel_mpgd_in_hitsXY ="MPGDBarrelHits.position.y*0.1:MPGDBarrelHits.position.x*0.1>>hitsxy_barrel_mm_in";
  TString tof_barrel_hitsXY ="TOFBarrelHits.position.y*0.1:TOFBarrelHits.position.x*0.1>>hitsxy_barrel_tof";
  TString barrel_mpgd_out_hitsXY ="OuterMPGDBarrelHits.position.y*0.1:OuterMPGDBarrelHits.position.x*0.1>>hitsxy_barrel_mm_out";

        sim->Draw(si_vtx_hitsXY.Data(),"",""); // Multiply by 0.1 for cm
        sim->Draw(si_barrel_hitsXY.Data(),"","");
        sim->Draw(barrel_mpgd_in_hitsXY.Data(),"","");
        sim->Draw(tof_barrel_hitsXY.Data(),"","");
        sim->Draw(barrel_mpgd_out_hitsXY.Data(),"","");

        TCanvas * c1 = new TCanvas("c1","coutput",1200,1000);
  c1->SetMargin(0.09, 0.03 ,0.1,0.06);
  c1->cd();
        hitsxy_vtx_si->SetMarkerStyle(31);
        hitsxy_vtx_si->SetTitle("Hit Points (XY)");
        hitsxy_vtx_si->SetMarkerColor(kBlack);
        hitsxy_vtx_si->SetMarkerSize(0.1);
        hitsxy_vtx_si->SetLineColor(kBlack);
        hitsxy_vtx_si->GetXaxis()->SetTitle("X [cm]");
        hitsxy_vtx_si->GetYaxis()->SetTitle("Y [cm]");
        hitsxy_vtx_si->GetXaxis()->CenterTitle();
        hitsxy_vtx_si->GetYaxis()->CenterTitle();

  hitsxy_barrel_si->SetMarkerStyle(20);
        hitsxy_barrel_si->SetMarkerSize(0.1);
        hitsxy_barrel_si->SetMarkerColor(kMagenta);
        hitsxy_barrel_si->SetLineColor(kMagenta);

  hitsxy_barrel_mm_in->SetMarkerStyle(20);
        hitsxy_barrel_mm_in->SetMarkerSize(0.1);
        hitsxy_barrel_mm_in->SetMarkerColor(kBlue);
        hitsxy_barrel_mm_in->SetLineColor(kBlue);

  hitsxy_barrel_tof->SetMarkerStyle(20);
        hitsxy_barrel_tof->SetMarkerSize(0.1);
        hitsxy_barrel_tof->SetMarkerColor(kGreen);
        hitsxy_barrel_tof->SetLineColor(kGreen);hitsxy_barrel_mm_out->SetMarkerStyle(20);
        hitsxy_barrel_mm_out->SetMarkerSize(0.1);
        hitsxy_barrel_mm_out->SetMarkerColor(kBlue-7);
        hitsxy_barrel_mm_out->SetLineColor(kBlue-7);
  hitsxy_vtx_si->Draw();
  hitsxy_barrel_si->Draw("same");
  hitsxy_barrel_mm_in->Draw("same");
  hitsxy_barrel_tof->Draw("same");
        hitsxy_barrel_mm_out->Draw("same");

 TLegend *l= new TLegend(0.65,0.85,0.90,1.0);
 l->SetTextSize(0.025);
 l->SetBorderSize(0);
 l->AddEntry(hitsxy_vtx_si,"VertexBarrelHits");
 l->AddEntry(hitsxy_barrel_si,"SiBarrelHits");
 l->AddEntry(hitsxy_barrel_mm_in,"MPGDBarrelHits");
 l->AddEntry(hitsxy_barrel_tof,"TOFBarrelHits");
 l->AddEntry(hitsxy_barrel_mm_out,"OuterMPGDBarrelHits");
 l->Draw();
 c1->SaveAs("hitsxy_dd4hep.png");

 // Y-Z Hits
 Int_t nbinsx = 400, nbinsy=1800.;
 x= 200.; y = 90.;
 double xmin = -200.;

   TH2D *hitsrz_vtx_si = new TH2D("hitsrz_vtx_si","hitsrz_vtx_si",nbinsx,xmin,x,nbinsy,-y,y);
   TH2D *hitsrz_barrel_si = new TH2D("hitsrz_barrel_si","hitsrz_barrel_si",nbinsx,-x,x,nbinsy,-y,y);
   TH2D *hitsrz_barrel_mm_in = new TH2D("hitsrz_barrel_mm_in","hitsrz_barrel_mm_in",nbinsx,-x,x,nbinsy,-y,y);
   TH2D *hitsrz_barrel_mm_out = new TH2D("hitsrz_barrel_mm_out","hitsrz_barrel_mm_out",nbinsx,-x,x,nbinsy,-y,y);
   TH2D *hitsrz_barrel_tof = new TH2D("hitsrz_barrel_tof","hitsrz_barrel_tof",nbinsx,-x,x,nbinsy,-y,y);
   TH2D *hitsrz_disks_si = new TH2D("hitsrz_disks_si","hitsrz_disks_si",nbinsx,-x,x,nbinsy,-y,y);
   TH2D *hitsrz_endcap_tof = new TH2D("hitsrz_endcap_tof","hitsrz_endcap_tof",nbinsx,-x,x,nbinsy,-y,y);
   TH2D *hitsrz_fwd_mpgd = new TH2D("hitsrz_fwd_mpgd","hitsrz_fwd_mpgd",nbinsx,-x,x,nbinsy,-y,y);
   TH2D *hitsrz_bwd_mpgd = new TH2D("hitsrz_bwd_mpgd","hitsrz_bwd_mpgd",nbinsx,-x,x,nbinsy,-y,y);

   TH2D *hitsrz_vtx_si_1 = new TH2D("hitsrz_vtx_si_1","hitsrz_vtx_si_1",nbinsx,xmin,x,nbinsy,-y,y);
   TH2D *hitsrz_barrel_si_1 = new TH2D("hitsrz_barrel_si_1","hitsrz_barrel_si_1",nbinsx,-x,x,nbinsy,-y,y);
   TH2D *hitsrz_barrel_mm_in_1 = new TH2D("hitsrz_barrel_mm_in_1","hitsrz_barrel_mm_in_1",nbinsx,-x,x,nbinsy,-y,y);
   TH2D *hitsrz_barrel_mm_out_1 = new TH2D("hitsrz_barrel_mm_out_1","hitsrz_barrel_mm_out_1",nbinsx,-x,x,nbinsy,-y,y);
   TH2D *hitsrz_barrel_tof_1 = new TH2D("hitsrz_barrel_tof_1","hitsrz_barrel_tof_1",nbinsx,-x,x,nbinsy,-y,y);
   TH2D *hitsrz_disks_si_1 = new TH2D("hitsrz_disks_si_1","hitsrz_disks_si_1",nbinsx,-x,x,nbinsy,-y,y);
   TH2D *hitsrz_endcap_tof_1 = new TH2D("hitsrz_endcap_tof_1","hitsrz_endcap_tof_1",nbinsx,-x,x,nbinsy,-y,y);
   TH2D *hitsrz_fwd_mpgd_1 = new TH2D("hitsrz_fwd_mpgd_1","hitsrz_fwd_mpgd_1",nbinsx,-x,x,nbinsy,-y,y);
   TH2D *hitsrz_bwd_mpgd_1 = new TH2D("hitsrz_bwd_mpgd_1","hitsrz_bwd_mpgd_1",nbinsx,-x,x,nbinsy,-y,y);

  TString si_vtx_hitsrz_posR  ="sqrt(VertexBarrelHits.position.x*VertexBarrelHits.position.x+VertexBarrelHits.position.y*VertexBarrelHits.position.y)*0.1:VertexBarrelHits.position.z*0.1>>hitsrz_vtx_si";

  TString si_barrel_hitsrz_posR ="sqrt(SiBarrelHits.position.x*SiBarrelHits.position.x+SiBarrelHits.position.y*SiBarrelHits.position.y)*0.1:SiBarrelHits.position.z*0.1>>hitsrz_barrel_si";

  TString barrel_mpgd_in_hitsrz_posR= "sqrt(MPGDBarrelHits.position.x*MPGDBarrelHits.position.x+MPGDBarrelHits.position.y*MPGDBarrelHits.position.y)*0.1:MPGDBarrelHits.position.z*0.1>>hitsrz_barrel_mm_in";

  TString tof_barrel_hitsrz_posR ="sqrt(TOFBarrelHits.position.x*TOFBarrelHits.position.x+TOFBarrelHits.position.y*TOFBarrelHits.position.y)*0.1:TOFBarrelHits.position.z*0.1>>hitsrz_barrel_tof";

  TString barrel_mpgd_out_hitsrz_posR ="sqrt(OuterMPGDBarrelHits.position.x*OuterMPGDBarrelHits.position.x+OuterMPGDBarrelHits.position.y*OuterMPGDBarrelHits.position.y)*0.1:OuterMPGDBarrelHits.position.z*0.1>>hitsrz_barrel_mm_out";

    TString disks_si_hitsrz_posR ="sqrt(TrackerEndcapHits.position.x*TrackerEndcapHits.position.x+TrackerEndcapHits.position.y*TrackerEndcapHits.position.y)*0.1:TrackerEndcapHits.position.z*0.1>>hitsrz_disks_si";

    TString endcap_tof_hitsrz_posR ="sqrt(TOFEndcapHits.position.x*TOFEndcapHits.position.x+TOFEndcapHits.position.y*TOFEndcapHits.position.y)*0.1:TOFEndcapHits.position.z*0.1>>hitsrz_endcap_tof";

    TString fwd_mpgd_hitsrz_posR ="sqrt(ForwardMPGDEndcapHits.position.x*ForwardMPGDEndcapHits.position.x+ForwardMPGDEndcapHits.position.y*ForwardMPGDEndcapHits.position.y)*0.1:ForwardMPGDEndcapHits.position.z*0.1>>hitsrz_fwd_mpgd";

    TString bwd_mpgd_hitsrz_posR ="sqrt(BackwardMPGDEndcapHits.position.x*BackwardMPGDEndcapHits.position.x+BackwardMPGDEndcapHits.position.y*BackwardMPGDEndcapHits.position.y)*0.1:BackwardMPGDEndcapHits.position.z*0.1>>hitsrz_bwd_mpgd";


        sim->Draw(si_vtx_hitsrz_posR.Data(),"VertexBarrelHits.position.y>0",""); // Multiply by 0.1 for cm
        sim->Draw(si_barrel_hitsrz_posR.Data(),"SiBarrelHits.position.y>0","");
        sim->Draw(barrel_mpgd_in_hitsrz_posR.Data(),"MPGDBarrelHits.position.y>0","");
        sim->Draw(tof_barrel_hitsrz_posR.Data(),"TOFBarrelHits.position.y>0","");
        sim->Draw(disks_si_hitsrz_posR.Data(),"TrackerEndcapHits.position.y>0","");
        sim->Draw(endcap_tof_hitsrz_posR.Data(),"TOFEndcapHits.position.y>0","");
  sim->Draw(barrel_mpgd_out_hitsrz_posR.Data(),"OuterMPGDBarrelHits.position.y>0","");
  sim->Draw(fwd_mpgd_hitsrz_posR.Data(),"ForwardMPGDEndcapHits.position.y>0","");
        sim->Draw(bwd_mpgd_hitsrz_posR.Data(),"BackwardMPGDEndcapHits.position.y>0","");

          TString si_vtx_hitsrz_negR  ="-1.0*sqrt(VertexBarrelHits.position.x*VertexBarrelHits.position.x+VertexBarrelHits.position.y*VertexBarrelHits.position.y)*0.1:VertexBarrelHits.position.z*0.1>>hitsrz_vtx_si_1";

  TString si_barrel_hitsrz_negR ="-1.0*sqrt(SiBarrelHits.position.x*SiBarrelHits.position.x+SiBarrelHits.position.y*SiBarrelHits.position.y)*0.1:SiBarrelHits.position.z*0.1>>hitsrz_barrel_si_1";

  TString barrel_mpgd_in_hitsrz_negR= "-1.0*sqrt(MPGDBarrelHits.position.x*MPGDBarrelHits.position.x+MPGDBarrelHits.position.y*MPGDBarrelHits.position.y)*0.1:MPGDBarrelHits.position.z*0.1>>hitsrz_barrel_mm_in_1";

  TString tof_barrel_hitsrz_negR ="-1.0*sqrt(TOFBarrelHits.position.x*TOFBarrelHits.position.x+TOFBarrelHits.position.y*TOFBarrelHits.position.y)*0.1:TOFBarrelHits.position.z*0.1>>hitsrz_barrel_tof_1";

  TString barrel_mpgd_out_hitsrz_negR ="-1.0*sqrt(OuterMPGDBarrelHits.position.x*OuterMPGDBarrelHits.position.x+OuterMPGDBarrelHits.position.y*OuterMPGDBarrelHits.position.y)*0.1:OuterMPGDBarrelHits.position.z*0.1>>hitsrz_barrel_mm_out_1";

    TString disks_si_hitsrz_negR ="-1.0*sqrt(TrackerEndcapHits.position.x*TrackerEndcapHits.position.x+TrackerEndcapHits.position.y*TrackerEndcapHits.position.y)*0.1:TrackerEndcapHits.position.z*0.1>>hitsrz_disks_si_1";

    TString endcap_tof_hitsrz_negR ="-1.0*sqrt(TOFEndcapHits.position.x*TOFEndcapHits.position.x+TOFEndcapHits.position.y*TOFEndcapHits.position.y)*0.1:TOFEndcapHits.position.z*0.1>>hitsrz_endcap_tof_1";

    TString fwd_mpgd_hitsrz_negR ="-1.0*sqrt(ForwardMPGDEndcapHits.position.x*ForwardMPGDEndcapHits.position.x+ForwardMPGDEndcapHits.position.y*ForwardMPGDEndcapHits.position.y)*0.1:ForwardMPGDEndcapHits.position.z*0.1>>hitsrz_fwd_mpgd_1";

    TString bwd_mpgd_hitsrz_negR ="-1.0*sqrt(BackwardMPGDEndcapHits.position.x*BackwardMPGDEndcapHits.position.x+BackwardMPGDEndcapHits.position.y*BackwardMPGDEndcapHits.position.y)*0.1:BackwardMPGDEndcapHits.position.z*0.1>>hitsrz_bwd_mpgd_1";

  sim->Draw(si_vtx_hitsrz_negR.Data(),"VertexBarrelHits.position.y<0",""); // Multiply by 0.1 for cm
        sim->Draw(si_barrel_hitsrz_negR.Data(),"SiBarrelHits.position.y<0","");
        sim->Draw(barrel_mpgd_in_hitsrz_negR.Data(),"MPGDBarrelHits.position.y<0","");
        sim->Draw(tof_barrel_hitsrz_negR.Data(),"TOFBarrelHits.position.y<0","");
        sim->Draw(disks_si_hitsrz_negR.Data(),"TrackerEndcapHits.position.y<0","");
        sim->Draw(endcap_tof_hitsrz_negR.Data(),"TOFEndcapHits.position.y<0","");
  sim->Draw(barrel_mpgd_out_hitsrz_negR.Data(),"OuterMPGDBarrelHits.position.y<0","");
  sim->Draw(fwd_mpgd_hitsrz_negR.Data(),"ForwardMPGDEndcapHits.position.y<0","");
        sim->Draw(bwd_mpgd_hitsrz_negR.Data(),"BackwardMPGDEndcapHits.position.y<0","");

  TCanvas * c2 = new TCanvas("c2","coutput",1200,1000);
  c2->SetMargin(0.09, 0.03 ,0.1,0.06);
  c2->cd();
  hitsrz_vtx_si->SetMarkerStyle(31);
        hitsrz_vtx_si->SetTitle("");
        hitsrz_vtx_si->SetMarkerSize(0.1);
        hitsrz_vtx_si->SetLineColor(kBlack);
        hitsrz_vtx_si->SetMarkerColor(kBlack);
        hitsrz_vtx_si->GetXaxis()->SetTitle("Z [cm]");
        hitsrz_vtx_si->GetYaxis()->SetTitle("R [cm]");
        hitsrz_vtx_si->GetXaxis()->CenterTitle();
        hitsrz_vtx_si->GetYaxis()->CenterTitle();

  hitsrz_vtx_si_1->SetMarkerSize(0.1);
        hitsrz_vtx_si_1->SetLineColor(kBlack);
        hitsrz_vtx_si_1->SetMarkerColor(kBlack);

        hitsrz_barrel_si->SetMarkerStyle(20);
        hitsrz_barrel_si->SetMarkerSize(0.1);
        hitsrz_barrel_si->SetMarkerColor(kMagenta);
        hitsrz_barrel_si->SetLineColor(kMagenta);

  hitsrz_barrel_si_1->SetMarkerSize(0.1);
        hitsrz_barrel_si_1->SetLineColor(kMagenta);
        hitsrz_barrel_si_1->SetMarkerColor(kMagenta);

        hitsrz_barrel_mm_in->SetMarkerStyle(20);
        hitsrz_barrel_mm_in->SetMarkerSize(0.1);
        hitsrz_barrel_mm_in->SetLineColor(kBlue);
        hitsrz_barrel_mm_in->SetMarkerColor(kBlue);
  hitsrz_barrel_mm_in_1->SetMarkerSize(0.1);
        hitsrz_barrel_mm_in_1->SetLineColor(kBlue);
        hitsrz_barrel_mm_in_1->SetMarkerColor(kBlue);

        hitsrz_barrel_tof->SetMarkerStyle(20);
        hitsrz_barrel_tof->SetMarkerSize(0.1);
        hitsrz_barrel_tof->SetLineColor(kGreen);
        hitsrz_barrel_tof->SetMarkerColor(kGreen);
  hitsrz_barrel_tof_1->SetMarkerSize(0.1);
        hitsrz_barrel_tof_1->SetLineColor(kGreen);
        hitsrz_barrel_tof_1->SetMarkerColor(kGreen);


  hitsrz_barrel_mm_out->SetMarkerStyle(20);
        hitsrz_barrel_mm_out->SetMarkerSize(0.1);
        hitsrz_barrel_mm_out->SetMarkerColor(kBlue-7);
        hitsrz_barrel_mm_out->SetLineColor(kBlue-7);

  hitsrz_barrel_mm_out_1->SetMarkerSize(0.1);
        hitsrz_barrel_mm_out_1->SetLineColor(kBlue-7);
        hitsrz_barrel_mm_out_1->SetMarkerColor(kBlue-7);
  hitsrz_endcap_tof->SetMarkerStyle(20);
        hitsrz_endcap_tof->SetMarkerSize(0.1);
        hitsrz_endcap_tof->SetMarkerColor(kCyan);
        hitsrz_endcap_tof->SetLineColor(kCyan);

  hitsrz_endcap_tof_1->SetMarkerSize(0.1);
        hitsrz_endcap_tof_1->SetLineColor(kCyan);
        hitsrz_endcap_tof_1->SetMarkerColor(kCyan);

        hitsrz_disks_si->SetMarkerStyle(20);
        hitsrz_disks_si->SetMarkerSize(0.1);
        hitsrz_disks_si->SetMarkerColor(kRed);
        hitsrz_disks_si->SetLineColor(kRed);

  hitsrz_disks_si_1->SetMarkerSize(0.1);
        hitsrz_disks_si_1->SetLineColor(kRed);
        hitsrz_disks_si_1->SetMarkerColor(kRed);

  hitsrz_fwd_mpgd->SetMarkerSize(0.1);
        hitsrz_fwd_mpgd->SetLineColor(kRed-7);
        hitsrz_fwd_mpgd->SetMarkerColor(kRed-7);
        hitsrz_fwd_mpgd_1->SetMarkerSize(0.1);
        hitsrz_fwd_mpgd_1->SetLineColor(kRed-7);
        hitsrz_fwd_mpgd_1->SetMarkerColor(kRed-7);
        hitsrz_bwd_mpgd->SetMarkerSize(0.1);
        hitsrz_bwd_mpgd->SetLineColor(kOrange);
        hitsrz_bwd_mpgd->SetMarkerColor(kOrange);

  hitsrz_bwd_mpgd_1->SetMarkerSize(0.1);
        hitsrz_bwd_mpgd_1->SetLineColor(kOrange);
        hitsrz_bwd_mpgd_1->SetMarkerColor(kOrange);

        hitsrz_vtx_si->Draw();
        hitsrz_vtx_si_1->Draw("same");
        hitsrz_barrel_si->Draw("same");
        hitsrz_barrel_si_1->Draw("same");
        hitsrz_barrel_mm_in->Draw("same");
        hitsrz_barrel_mm_in_1->Draw("same");
        hitsrz_barrel_tof->Draw("same");
        hitsrz_barrel_tof_1->Draw("same");
        hitsrz_barrel_mm_out->Draw("same");
        hitsrz_barrel_mm_out_1->Draw("same");
        hitsrz_endcap_tof->Draw("same");
        hitsrz_endcap_tof_1->Draw("same");
        hitsrz_disks_si->Draw("same");
        hitsrz_disks_si_1->Draw("same");
        hitsrz_fwd_mpgd->Draw("same");
        hitsrz_fwd_mpgd_1->Draw("same");
        hitsrz_bwd_mpgd->Draw("same");
        hitsrz_bwd_mpgd_1->Draw("same");

  l= new TLegend(0.11,0.88,0.95,0.99);
  l->SetNColumns(3);
  l->SetTextSize(0.025);
  l->SetBorderSize(0);
  l->AddEntry(hitsrz_vtx_si,"VertexBarrelHits");
  l->AddEntry(hitsrz_barrel_si,"SiBarrelHits");
  l->AddEntry(hitsrz_barrel_mm_in,"MPGDBarrelHits");
  l->AddEntry(hitsrz_barrel_tof,"TOFBarrelHits");
  l->AddEntry(hitsrz_barrel_mm_out,"OuterMPGDBarrelHits");
  l->AddEntry(hitsrz_disks_si,"TrackerEndcapHits");
  l->AddEntry(hitsrz_endcap_tof,"TOFEndcapHits");
  l->AddEntry(hitsrz_fwd_mpgd,"ForwardMPGDEndcapHits");
  l->AddEntry(hitsrz_bwd_mpgd,"BackwardMPGDEndcapHits");
  l->Draw();

  c2->SaveAs("hitsrz_dd4hep.png");
 // Timer Stop
  timer.Stop();
  Double_t realtime = timer.RealTime();
  Double_t cputime = timer.CpuTime();
  printf("RealTime=%f seconds, CpuTime=%f seconds\n",realtime,cputime);

}
