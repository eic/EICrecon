#include <iostream>
#include <vector>
using namespace std;

// ========================================================================================
// Main function
void plot_hists(){
	// Variables that the user should specify
	TString input_fname = "eicrecon_outnowwithmuons.root";
	TString output_fname = "plot_hists_etarange_flat_butmuons.pdf";

	//Define Style
	gStyle->SetOptStat(0);
	gStyle->SetPadBorderMode(0);
	gStyle->SetFrameBorderMode(0);
	gStyle->SetFrameLineWidth(2);
	gStyle->SetLabelSize(0.035,"X");
	gStyle->SetLabelSize(0.035,"Y");
	gStyle->SetTitleXSize(0.04);
	gStyle->SetTitleXOffset(0.9);
	gStyle->SetTitleYSize(0.04);
	gStyle->SetTitleYOffset(0.9);

	// -----------------------------------------------
	//Get ROOT file and load histograms
	TFile *f = new TFile(input_fname);

	TH2 *h1a = (TH2*) f->Get("track_qa/h1a");
	TH1 *hchi2 = (TH1*) f->Get("track_qa/hchi2");
	TH1 *heta = (TH1*) f->Get("track_qa/heta");
	TH1 *hp = (TH1*) f->Get("track_qa/hp");
	TH1 *hpt = (TH1*) f->Get("track_qa/hpt");
	TH1 *hhits = (TH1*) f->Get("track_qa/hhits");
	TH1 *hNDF = (TH1*) f->Get("track_qa/hNDF");
	TH1 *hchi2_by_hits = (TH1*) f->Get("track_qa/hchi2_by_hits");
	TH1 *hchi2_by_NDF = (TH1*) f->Get("track_qa/hchi2_by_NDF");

	TH2 *hchi2_vs_eta = (TH2*) f->Get("track_qa/hchi2_vs_eta");
	TH2 *hchi2_vs_hits = (TH2*) f->Get("track_qa/hchi2_vs_hits");
	TH2 *hchi2_vs_hits_zoomed = (TH2*) f->Get("track_qa/hchi2_vs_hits_zoomed");
	// vector<TH2*> hchi2_vs_hits_etabins = (vector<TH2*>) f->Get("track_qa/hchi2_vs_hits_etabins");
	TH2 *hhits_vs_eta = (TH2*) f->Get("track_qa/hhits_vs_eta");
	TH2 *htracks_vs_eta = (TH2*) f->Get("track_qa/htracks_vs_eta");
	TH3 *heta_vs_p_vs_chi2 = (TH3*) f->Get("track_qa/heta_vs_p_vs_chi2");
	TH2 *hmeasptrack_vs_eta = (TH2*) f->Get("track_qa/hmeasptrack_vs_eta");
	TH2 *hmeasptrack_vs_hits = (TH2*) f->Get("track_qa/hmeasptrack_vs_hits");
	TH2 *hmeasptrack_vs_chi2perNDF = (TH2*) f->Get("track_qa/hmeasptrack_vs_chi2perNDF");
	TH2 *hmeasptrack_vs_calstates = (TH2*) f->Get("track_qa/hmeasptrack_vs_calstates");

	TH2 *hmeaschi2_vs_chi2 = (TH2*) f->Get("track_qa/hmeaschi2_vs_chi2");
	TH2 *hmeaschi2_vs_eta = (TH2*) f->Get("track_qa/hmeaschi2_vs_eta");
	TH2 *hmeaschi2_vs_hits = (TH2*) f->Get("track_qa/hmeaschi2_vs_hits");

	// -----------------------------------------------
	// Loading histograms broken down in eta bins
	TVectorT<double> * V_eta_edges = (TVectorT<double> *) f -> Get("V_eta_edges");
        const int num_eta_bin = (*V_eta_edges).GetNoElements()-1;

	vector<TH2*> hchi2_vs_hits_etabins(num_eta_bin);
        vector<TH2*> hmeasptrack_vs_hits_etabins(num_eta_bin);
        vector<TH2*> hmeasptrack_vs_hits_etabins_zoomed(num_eta_bin);
        vector<TH2*> hmeasptrack_vs_chi2perNDF_etabins(num_eta_bin);

	vector<TPaveText*> tex_eta(num_eta_bin);

	for (int i=0; i<num_eta_bin; i++){
		double low_eta = (*V_eta_edges)[i];
		double high_eta = (*V_eta_edges)[i+1];

		hchi2_vs_hits_etabins[i] = (TH2*) f->Get(Form("track_qa/eta_bins/hchi2_vs_hits_eta_%.1f_%.1f", low_eta, high_eta));
		hmeasptrack_vs_hits_etabins[i] = (TH2*) f->Get(Form("track_qa/eta_bins/hmeasptrack_vs_hits_eta_%.1f_%.1f", low_eta, high_eta));
		hmeasptrack_vs_hits_etabins_zoomed[i] = (TH2*) f->Get(Form("track_qa/eta_bins/hmeasptrack_vs_hits_eta_zoomed_%.1f_%.1f", low_eta, high_eta));
		hmeasptrack_vs_chi2perNDF_etabins[i] = (TH2*) f->Get(Form("track_qa/eta_bins/hmeasptrack_vs_chi2perNDF_eta_%.1f_%.1f", low_eta, high_eta));
	
		tex_eta[i] = new TPaveText(0.5,0.7,0.7,0.85,"NDCNB");
                tex_eta[i]->AddText(Form("%.1f < #eta < %.1f", low_eta, high_eta));
                tex_eta[i]->SetFillStyle(4000);tex_eta[i]->SetTextFont(63);tex_eta[i]->SetTextSize(10);
	}
 
	// -----------------------------------------------

	TH2 *hholes_vs_hits = (TH2*) f->Get("track_qa/hholes_vs_hits");
	TH2 *houtliers_vs_hits = (TH2*) f->Get("track_qa/houtliers_vs_hits");
	TH2 *hsummation = (TH2*) f->Get("track_qa/hsummation");
	TH2 *hsummation2 = (TH2*) f->Get("track_qa/hsummation2");

	// -----------------------------------------------

	auto fdiagline = new TF1("fdiagline","x",0,10);
        fdiagline->SetLineWidth(3);fdiagline->SetLineColor(kRed);
        auto fdiagline_thin = new TF1("fdiagline_thin","x",0,10);
        fdiagline_thin->SetLineWidth(2);fdiagline_thin->SetLineColor(kRed);

	// -----------------------------------------------
	//Make plots
	const int nCanvas = 25;
	TCanvas ** c1 = new TCanvas * [nCanvas];

	c1[0] = new TCanvas("c0","c0");
	h1a->Draw("colz");

	TPaveText* tex_gen = new TPaveText(0.2,0.7,0.5,0.85,"NDCNB");
	tex_gen->AddText("Single #mu^{-} generated:");
	tex_gen->AddText("0.5 GeV/c < P < 20 GeV/c");
	tex_gen->AddText("-4 < #eta < 4, 0^{o} < #phi < 360^{o}");
	tex_gen->SetFillStyle(4000);tex_gen->SetTextFont(63);tex_gen->SetTextSize(20);
	tex_gen->Draw();

	TPaveText* tex_zoom = new TPaveText(0.7,0.9,0.8,0.95,"NDCNB");
	tex_zoom->AddText("Zoomed in");
	tex_zoom->SetFillStyle(4000);tex_zoom->SetTextFont(63);tex_zoom->SetTextSize(20);tex_zoom->SetTextColor(kRed);

	c1[1] = new TCanvas("c1","c1");
	hchi2->Draw();
	tex_gen->Draw();

	c1[2] = new TCanvas("c2","c2");
	c1[2]->Divide(2,2);
	c1[2]->cd(1);
	hhits->Draw();
	c1[2]->cd(2);
	hNDF->Draw();
	c1[2]->cd(3);
	hchi2_by_hits->Draw();
	c1[2]->cd(4);
	hchi2_by_NDF->Draw();
	c1[2]->Modified();

	c1[3] = new TCanvas("c3","c3",1200,900);
	c1[3]->Divide(2,2);
	c1[3]->cd(1);
	heta->Draw();
	c1[3]->cd(2);
	hp->Draw();
	c1[3]->cd(3);
	hpt->Draw();

	c1[4] = new TCanvas("c4","c4");
	hchi2_vs_eta->Draw("colz");

	c1[5] = new TCanvas("c5","c5");
	hchi2_vs_hits->Draw("colz");

	c1[6] = new TCanvas("c6","c6");
	hchi2_vs_hits_zoomed->Draw("colz");

	c1[7] = new TCanvas("c7","c7");
	c1[7]->Divide(4,4);
	for (int i=0; i<num_eta_bin; i++){
		c1[7]->cd(i+1);
		hchi2_vs_hits_etabins[i]->Draw("colz");
		tex_eta[i]->Draw();
	}

	c1[8] = new TCanvas("c8","c8");
	hhits_vs_eta->Draw("colz");

	c1[9] = new TCanvas("c9","c9");
	htracks_vs_eta->Draw("colz");

	c1[10] = new TCanvas("c10","c10");
	heta_vs_p_vs_chi2->Draw("lego2");

	c1[11] = new TCanvas("c11","c11");
	hmeasptrack_vs_eta->Draw("colz");

	c1[12] = new TCanvas("c12","c12");
	hmeasptrack_vs_hits->Draw("colz");
	fdiagline->Draw("same");

	c1[13] = new TCanvas("c13","c13");
	c1[13]->Divide(4,4);
	for (int i=0; i<num_eta_bin; i++){
		c1[13]->cd(i+1);
		hmeasptrack_vs_hits_etabins[i]->Draw("colz");
		tex_eta[i]->Draw();
		fdiagline_thin->Draw("same");
	}

	c1[14] = new TCanvas("c14","c14");
	c1[14]->Divide(4,4);
	for (int i=0; i<num_eta_bin; i++){
		c1[14]->cd(i+1);
		hmeasptrack_vs_hits_etabins_zoomed[i]->Draw("colz");
		tex_eta[i]->Draw();
		fdiagline_thin->Draw("same");
	}

	c1[15] = new TCanvas("c15","c15");
	hmeasptrack_vs_chi2perNDF->Draw("colz");

	c1[16] = new TCanvas("c16","c16");
	c1[16]->Divide(4,4);
	for (int i=0; i<num_eta_bin; i++){
		c1[16]->cd(i+1);
		hmeasptrack_vs_chi2perNDF_etabins[i]->Draw("colz");
		tex_eta[i]->Draw();
	}

	c1[17] = new TCanvas("c17","c17");
	hmeasptrack_vs_calstates->Draw("colz");

	c1[18] = new TCanvas("c18","c18");
	hmeaschi2_vs_chi2->Draw("colz");

	c1[19] = new TCanvas("c19","c19");
	hmeaschi2_vs_eta->Draw("colz");

	c1[20] = new TCanvas("c20","c20");
	hmeaschi2_vs_hits->Draw("colz");

	c1[21] = new TCanvas("c21","c21");
	hholes_vs_hits->Draw("colz");

	c1[22] = new TCanvas("c22","c22");
	houtliers_vs_hits->Draw("colz");

	c1[23] = new TCanvas("c23","c23");
	hsummation->Draw("colz");

	c1[24] = new TCanvas("c24","c24");
	hsummation2->Draw("colz");

	for(int i = 0 ; i < nCanvas ; i++){
		if(i==0) c1[i]->Print(output_fname+"(");
		else if (i==nCanvas-1) c1[i]->Print(output_fname+")");
		else c1[i]->Print(output_fname);
	}
	
}

