// Hits distribuition of detectors
// Shyam Kumar:INFN Bari, shyam.kumar@ba.infn.it; shyam055119@gmail.com

#include <TGraphErrors.h>
#include <TF1.h>
#include <TRandom.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TMath.h>

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

 TCanvas * c1 = new TCanvas("c1","coutput",1200,1000);
 c1->SetMargin(0.09, 0.03 ,0.1,0.06);

 TCanvas * c2 = new TCanvas("c2","coutput",1200,1000);
 c2->SetMargin(0.09, 0.03 ,0.1,0.06);
 // X-Y Hits
 Int_t nbins = 320;
 Double_t x= 100., y = 100.;
 TH2D *h1 = new TH2D("h1","h1",nbins,-x,x,nbins,-y,y);
 TH2D *h2 = new TH2D("h2","h2",nbins,-x,x,nbins,-y,y);
 TH2D *h3 = new TH2D("h3","h3",nbins,-x,x,nbins,-y,y);
 TH2D *h4 = new TH2D("h4","h4",nbins,-x,x,nbins,-y,y);
 TH2D *h5 = new TH2D("h5","h5",nbins,-x,x,nbins,-y,y);

 // Y-Z Hits
 Int_t nbinsx = 400, nbinsy=180.;
 x= 200.; y = 90.;
 double xmin = -200.;
 TH2D *h1_1 = new TH2D("h1_1","h1_1",nbinsx,xmin,x,nbinsy,-y,y);
 TH2D *h2_1 = new TH2D("h2_1","h2_1",nbinsx,xmin,x,nbinsy,-y,y);
 TH2D *h3_1 = new TH2D("h3_1","h3_1",nbinsx,xmin,x,nbinsy,-y,y);
 TH2D *h4_1 = new TH2D("h4_1","h4_1",nbinsx,xmin,x,nbinsy,-y,y);
 TH2D *h5_1 = new TH2D("h5_1","h5_1",nbinsx,xmin,x,nbinsy,-y,y);
 TH2D *h6_1 = new TH2D("h6_1","h6_1",nbinsx,xmin,x,nbinsy,-y,y);

 TH2D *h1_2 = new TH2D("h1_2","h1_2",nbinsx,xmin,x,nbinsy,-y,y);
 TH2D *h2_2 = new TH2D("h2_2","h2_2",nbinsx,xmin,x,nbinsy,-y,y);
 TH2D *h3_2 = new TH2D("h3_2","h3_2",nbinsx,xmin,x,nbinsy,-y,y);
 TH2D *h4_2 = new TH2D("h4_2","h4_2",nbinsx,xmin,x,nbinsy,-y,y);
 TH2D *h5_2 = new TH2D("h5_2","h5_2",nbinsx,xmin,x,nbinsy,-y,y);
 TH2D *h6_2 = new TH2D("h6_2","h6_2",nbinsx,xmin,x,nbinsy,-y,y);



//////////////////////////////////////////////////////////////////////
	c1->cd();
	sim->Draw("VertexBarrelHits.position.y*0.1:VertexBarrelHits.position.x*0.1>>h1","",""); // Multiply by 0.1 for cm
	sim->Draw("SiBarrelHits.position.y*0.1:SiBarrelHits.position.x*0.1>>h2","","");
	sim->Draw("MPGDBarrelHits.position.y*0.1:MPGDBarrelHits.position.x*0.1>>h3","","");
	sim->Draw("TOFBarrelHits.position.y*0.1:TOFBarrelHits.position.x*0.1>>h4","","");
	//sim->Draw("TrackerEndcapHits.position.y*0.1:TrackerEndcapHits.position.x*0.1>>h5","","");
	h1->SetMarkerStyle(31);
	h1->SetTitle("Hit Points");
	h1->SetMarkerColor(kBlack);
	h1->SetMarkerSize(0.1);
	h1->SetLineColor(kBlack);
	h1->GetXaxis()->SetTitle("X [cm]");
	h1->GetYaxis()->SetTitle("Y [cm]");
	h1->GetXaxis()->CenterTitle();
	h1->GetYaxis()->CenterTitle();
	h1->Draw();

     h2->SetMarkerStyle(20);
	h2->SetMarkerSize(0.1);
	h2->SetMarkerColor(kMagenta);
	h2->SetLineColor(kMagenta);
	h2->Draw("same");

     h3->SetMarkerStyle(20);
	h3->SetMarkerSize(0.1);
	h3->SetMarkerColor(kBlue);
	h3->SetLineColor(kBlue);
	h3->Draw("same");

     h4->SetMarkerStyle(20);
	h4->SetMarkerSize(0.1);
	h4->SetMarkerColor(kGreen);
	h4->SetLineColor(kGreen);
	h4->Draw("same");

	h5->SetMarkerStyle(20);
	h5->SetMarkerSize(0.1);
	h5->SetMarkerColor(kRed);
	h5->SetLineColor(kRed);
	h5->Draw("same");

 TLegend *l= new TLegend(0.65,0.85,0.90,1.0);
 l->SetTextSize(0.03);
 l->SetBorderSize(0);
 l->AddEntry(h1,"VertexBarrelHits");
 l->AddEntry(h2,"SiBarrelHits");
 l->AddEntry(h3,"MPGDBarrelHits");
 l->AddEntry(h4,"TOFBarrelHits");
// l->AddEntry(h5,"DIRC Hits");
 c1->cd();
 l->Draw();

     c2->cd();
	sim->Draw("sqrt(VertexBarrelHits.position.x*0.1*VertexBarrelHits.position.x*0.1+VertexBarrelHits.position.y*0.1*VertexBarrelHits.position.y*0.1):VertexBarrelHits.position.z*0.1>>h1_1","VertexBarrelHits.position.y>0",""); // Multiply by 0.1 for cm
	sim->Draw("sqrt(SiBarrelHits.position.x*0.1*SiBarrelHits.position.x*0.1+SiBarrelHits.position.y*0.1*SiBarrelHits.position.y*0.1):SiBarrelHits.position.z*0.1>>h2_1","SiBarrelHits.position.y>0","");
	sim->Draw("sqrt(MPGDBarrelHits.position.x*0.1*MPGDBarrelHits.position.x*0.1+MPGDBarrelHits.position.y*0.1*MPGDBarrelHits.position.y*0.1):MPGDBarrelHits.position.z*0.1>>h3_1","MPGDBarrelHits.position.y>0","");
	sim->Draw("sqrt(TOFBarrelHits.position.x*0.1*TOFBarrelHits.position.x*0.1+TOFBarrelHits.position.y*0.1*TOFBarrelHits.position.y*0.1):TOFBarrelHits.position.z*0.1>>h4_1","TOFBarrelHits.position.y>0","");
	sim->Draw("sqrt(TrackerEndcapHits.position.x*0.1*TrackerEndcapHits.position.x*0.1+TrackerEndcapHits.position.y*0.1*TrackerEndcapHits.position.y*0.1):TrackerEndcapHits.position.z*0.1>>h5_1","TrackerEndcapHits.position.y>0","");
	sim->Draw("sqrt(TOFEndcapHits.position.x*0.1*TOFEndcapHits.position.x*0.1+TOFEndcapHits.position.y*0.1*TOFEndcapHits.position.y*0.1):TOFEndcapHits.position.z*0.1>>h6_1","TOFEndcapHits.position.y>0","");

     sim->Draw("-1.0*sqrt(VertexBarrelHits.position.x*0.1*VertexBarrelHits.position.x*0.1+VertexBarrelHits.position.y*0.1*VertexBarrelHits.position.y*0.1):VertexBarrelHits.position.z*0.1>>h1_2","VertexBarrelHits.position.y<0",""); // Multiply by 0.1 for cm
	sim->Draw("-1.0*sqrt(SiBarrelHits.position.x*0.1*SiBarrelHits.position.x*0.1+SiBarrelHits.position.y*0.1*SiBarrelHits.position.y*0.1):SiBarrelHits.position.z*0.1>>h2_2","SiBarrelHits.position.y<0","");
	sim->Draw("-1.0*sqrt(MPGDBarrelHits.position.x*0.1*MPGDBarrelHits.position.x*0.1+MPGDBarrelHits.position.y*0.1*MPGDBarrelHits.position.y*0.1):MPGDBarrelHits.position.z*0.1>>h3_2","MPGDBarrelHits.position.y<0","");
	sim->Draw("-1.0*sqrt(TOFBarrelHits.position.x*0.1*TOFBarrelHits.position.x*0.1+TOFBarrelHits.position.y*0.1*TOFBarrelHits.position.y*0.1):TOFBarrelHits.position.z*0.1>>h4_2","TOFBarrelHits.position.y<0","");
	sim->Draw("-1.0*sqrt(TrackerEndcapHits.position.x*0.1*TrackerEndcapHits.position.x*0.1+TrackerEndcapHits.position.y*0.1*TrackerEndcapHits.position.y*0.1):TrackerEndcapHits.position.z*0.1>>h5_2","TrackerEndcapHits.position.y<0","");
	sim->Draw("-1.0*sqrt(TOFEndcapHits.position.x*0.1*TOFEndcapHits.position.x*0.1+TOFEndcapHits.position.y*0.1*TOFEndcapHits.position.y*0.1):TOFEndcapHits.position.z*0.1>>h6_2","TOFEndcapHits.position.y<0","");

	h1_1->SetMarkerStyle(31);
	h1_1->SetTitle("Hit Points");
	h1_1->SetMarkerSize(0.1);
	h1_1->SetLineColor(kBlack);
	h1_1->SetMarkerColor(kBlack);
	h1_1->GetXaxis()->SetTitle("Z [cm]");
	h1_1->GetYaxis()->SetTitle("R [cm]");
	h1_1->GetXaxis()->CenterTitle();
	h1_1->GetYaxis()->CenterTitle();
	h1_1->Draw();
     h1_2->SetMarkerSize(0.1);
	h1_2->SetLineColor(kBlack);
	h1_2->SetMarkerColor(kBlack);
	h1_2->Draw("same");


	h2_1->SetMarkerStyle(20);
	h2_1->SetMarkerSize(0.1);
	h2_1->SetMarkerColor(kMagenta);
	h2_1->SetLineColor(kMagenta);
	h2_1->Draw("same");

     h2_2->SetMarkerSize(0.1);
	h2_2->SetLineColor(kMagenta);
	h2_2->SetMarkerColor(kMagenta);
	h2_2->Draw("same");


	h3_1->SetMarkerStyle(20);
	h3_1->SetMarkerSize(0.1);
	h3_1->SetLineColor(kBlue);
	h3_1->SetMarkerColor(kBlue);
	h3_1->Draw("same");

	h3_2->SetMarkerSize(0.1);
	h3_2->SetLineColor(kBlue);
	h3_2->SetMarkerColor(kBlue);
	h3_2->Draw("same");


	h4_1->SetMarkerStyle(20);
	h4_1->SetMarkerSize(0.1);
	h4_1->SetLineColor(kGreen);
	h4_1->SetMarkerColor(kGreen);
	h4_1->Draw("same");

	h4_2->SetMarkerSize(0.1);
	h4_2->SetLineColor(kGreen);
	h4_2->SetMarkerColor(kGreen);
	h4_2->Draw("same");

     h5_1->SetMarkerStyle(20);
	h5_1->SetMarkerSize(0.1);
	h5_1->SetMarkerColor(kRed);
	h5_1->SetLineColor(kRed);
	h5_1->Draw("same");

     h5_2->SetMarkerSize(0.1);
	h5_2->SetLineColor(kRed);
	h5_2->SetMarkerColor(kRed);
	h5_2->Draw("same");

	h6_1->SetMarkerStyle(20);
	h6_1->SetMarkerSize(0.1);
	h6_1->SetMarkerColor(kCyan);
	h6_1->SetLineColor(kCyan);
	h6_1->Draw("same");

     h6_2->SetMarkerSize(0.1);
	h6_2->SetLineColor(kCyan);
	h6_2->SetMarkerColor(kCyan);
	h6_2->Draw("same");


 l= new TLegend(0.64,0.87,0.85,1.0);
 l->SetTextSize(0.03);
 l->SetBorderSize(0);
 l->AddEntry(h1_1,"VertexBarrelHits");
 l->AddEntry(h2_1,"SiBarrelHits");
 l->AddEntry(h3_1,"MPGDBarrelHits");
 l->AddEntry(h4_1,"TOFBarrelHits");
 l->AddEntry(h5_1,"TrackerEndcapHits");
 l->AddEntry(h6_1,"TOFEndcapHits");
 c2->cd();
 l->Draw();

 c1->SaveAs("hitsxy_dd4hep.png");
 c2->SaveAs("hitsrz_dd4hep.png");


}
