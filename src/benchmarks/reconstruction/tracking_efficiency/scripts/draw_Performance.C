///////////// code for checking the basic things works in ALICE and also in PANDA of particles by shyam kumar

#include "TGraphErrors.h"
#include "TF1.h"
#include "TRandom.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TMath.h"

void draw_Performance(Int_t nevent=-1)
{

//==========Style of the plot============
   gStyle->SetPalette(1);
   gStyle->SetOptTitle(1);
   gStyle->SetTitleOffset(.85,"X");gStyle->SetTitleOffset(.85,"Y");
   gStyle->SetTitleSize(.04,"X");gStyle->SetTitleSize(.04,"Y");
   gStyle->SetLabelSize(.04,"X");gStyle->SetLabelSize(.04,"Y");
   gStyle->SetHistLineWidth(2);
   gStyle->SetOptFit(1);
   gStyle->SetOptStat(1);

//=======Reading the root file DD4HEP===========
   TFile* file = new TFile("tracking_test_gun.edm4eic.root"); // Tree with tracks and hits
		// Create the tree reader and its data containers
   TTreeReader myReader("events", file); // name of tree and file

   TTreeReaderArray<Float_t> charge(myReader, "MCParticles.charge");
   TTreeReaderArray<Double_t> vx_mc(myReader, "MCParticles.vertex.x");
   TTreeReaderArray<Double_t> vy_mc(myReader, "MCParticles.vertex.y");
   TTreeReaderArray<Double_t> vz_mc(myReader, "MCParticles.vertex.z");
   TTreeReaderArray<Float_t> px_mc(myReader, "MCParticles.momentum.x");
   TTreeReaderArray<Float_t> py_mc(myReader, "MCParticles.momentum.y");
   TTreeReaderArray<Float_t> pz_mc(myReader, "MCParticles.momentum.z");
   TTreeReaderArray<Int_t> status(myReader, "MCParticles.generatorStatus");
   TTreeReaderArray<Int_t> pdg(myReader, "MCParticles.PDG");

   TTreeReaderArray<Float_t> px_rec(myReader, "ReconstructedChargedParticles.momentum.x");
   TTreeReaderArray<Float_t> py_rec(myReader, "ReconstructedChargedParticles.momentum.y");
   TTreeReaderArray<Float_t> pz_rec(myReader, "ReconstructedChargedParticles.momentum.z");


 TCanvas * c[5];
 for (int i =0; i<5; ++i){
 c[i] = new TCanvas(Form("c%d",i),Form("c%d",i),1200,1000);
 c[i]->SetMargin(0.09, 0.1 ,0.1,0.06);
 }

 // X-Y Hits

 Int_t nbins = 200;
 Double_t x= 4.0;
 TH2D *hetavspmc = new TH2D("hetavspmc","hetavspmc",120,0.,12.,nbins,-x,x);
 TH2D *hetavsprec = new TH2D("hetavsprec","hetavsprec",120,0.,12.,nbins,-x,x);

 TH2D *hetavsptmc = new TH2D("hetavsptmc","hetavsptmc",120,0.,12.,nbins,-x,x);
 TH2D *hetavsptrec = new TH2D("hetavsptrec","hetavsptrec",120,0.,12.,nbins,-x,x);

 TH1D *hpresol = new TH1D("hpresol","hpresol",200,-0.0001,0.0001);

 hetavspmc->GetXaxis()->SetTitle("p_{mc} (GeV/c)");
 hetavspmc->GetYaxis()->SetTitle("#eta_{mc}");
 hetavspmc->GetXaxis()->CenterTitle();
 hetavspmc->GetYaxis()->CenterTitle();

 hetavsptmc->GetXaxis()->SetTitle("pt_{mc} (GeV/c)");
 hetavsptmc->GetYaxis()->SetTitle("#eta_{mc}");
 hetavsptmc->GetXaxis()->CenterTitle();
 hetavsptmc->GetYaxis()->CenterTitle();

 hetavsprec->GetXaxis()->SetTitle("p_{rec} (GeV/c)");
 hetavsprec->GetYaxis()->SetTitle("#eta_{rec}");
 hetavsprec->GetXaxis()->CenterTitle();
 hetavsprec->GetYaxis()->CenterTitle();

 hetavsptrec->GetXaxis()->SetTitle("pt_{rec} (GeV/c)");
 hetavsptrec->GetYaxis()->SetTitle("#eta_{rec}");
 hetavsptrec->GetXaxis()->CenterTitle();
 hetavsptrec->GetYaxis()->CenterTitle();

 hpresol->GetXaxis()->SetTitle("dp/p");
 hpresol->GetYaxis()->SetTitle("Entries (a.u.)");
 hpresol->GetXaxis()->CenterTitle();
 hpresol->GetYaxis()->CenterTitle();


//////////////////////////////////////////////////////////////////////
 int count = 0;
  while (myReader.Next())
  {
   if (nevent>0 && count>nevent) continue;
 //  cout<<"=====Event No. "<<count<<"============="<<endl;
     Double_t pmc = 0;
   // MC Particle
     for (int iParticle = 0; iParticle < charge.GetSize(); ++iParticle){
   //  cout<<" PDG: "<<pdg[iParticle]<<" Status: "<<status[iParticle]<<" Pt: "<<sqrt(px_mc[iParticle]*px_mc[iParticle]+py_mc[iParticle]*py_mc[iParticle])<<endl;
     if (status[iParticle] ==1){
     pmc = sqrt(px_mc[iParticle]*px_mc[iParticle]+py_mc[iParticle]*py_mc[iParticle]+pz_mc[iParticle]*pz_mc[iParticle]);
     Double_t ptmc = sqrt(px_mc[iParticle]*px_mc[iParticle]+py_mc[iParticle]*py_mc[iParticle]);
     Double_t etamc = -1.0*TMath::Log(TMath::Tan((TMath::ACos(pz_mc[iParticle]/pmc))/2));
     hetavspmc->Fill(pmc,etamc);
     hetavsptmc->Fill(ptmc,etamc);
     }
     }

    // Rec Particle
     for (int iRecParticle = 0; iRecParticle < px_rec.GetSize(); ++iRecParticle){
     Double_t prec = sqrt(px_rec[iRecParticle]*px_rec[iRecParticle]+py_rec[iRecParticle]*py_rec[iRecParticle]+pz_rec[iRecParticle]*pz_rec[iRecParticle]);
     Double_t ptrec = sqrt(px_rec[iRecParticle]*px_rec[iRecParticle]+py_rec[iRecParticle]*py_rec[iRecParticle]);
     Double_t etarec = -1.0*TMath::Log(TMath::Tan((TMath::ACos(pz_rec[iRecParticle]/prec))/2));
     hetavsprec->Fill(prec,etarec);
     hetavsptrec->Fill(ptrec,etarec);
     hpresol->Fill((prec-pmc)/pmc);

     }

    count++;

  }

     c[0]->cd();
     hetavspmc->Draw("colz");
     c[0]->SaveAs("eta_mcvspmc.png");
     c[1]->cd();
     hetavsprec->Draw("colz");
     c[1]->SaveAs("eta_mcvsprec.png");
     c[2]->cd();
     hetavsptmc->Draw("colz");
     c[2]->SaveAs("eta_mcvsptmc.png");
     c[3]->cd();
     hetavsptrec->Draw("colz");
     c[3]->SaveAs("eta_mcvsptrec.png");

     c[4]->cd();
     hpresol->Draw("hist");
     c[4]->SaveAs("ptresol.png");
}
