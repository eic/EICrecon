#include <iostream>
#include <vector>
using namespace std;

// customize tex label
TPaveText* makeTexLabel(TString text, double x1=0.2, double y1=0.7, double x2=0.5, double y2=0.85,
						int textsize=20, int textcolor=kBlack, int textfont=63){
							//potentially make text an array
	TPaveText* tex_label = new TPaveText(x1,y1,x2,y2,"NDCNB");
	tex_label->AddText(text);
	tex_label->SetFillStyle(4000);
	tex_label->SetTextFont(textfont);
	tex_label->SetTextSize(textsize);
	tex_label->SetTextColor(textcolor);

	return tex_label;
}

// ========================================================================================
// Make gaussian fits
Double_t makeFits(TH1* hist){ //(vector<TH1F*> hists, Double_t* st_dev=NULL){

    double lowerbound = -0.5;
    double upperbound = 0.5;

    Double_t st_dev = 0;
	TF1* g1 = new TF1("g1", "gaus",lowerbound,upperbound);

	hist->Fit(g1, "RQ");
	st_dev = g1->GetParameter(2);

    return st_dev;
}

double** makeDGFits(TH1* hist, double** fit_widths, int i, double param0=20., double param3=100.){
	TF1 *dgfit = new TF1("dgfit","gaus(0)+gaus(3)",-0.1,0.1);

	dgfit->SetParameter(0,param0);//SetParLimits(0,1.0,1e3);
	dgfit->SetParameter(1,hist->GetMean());
	dgfit->SetParLimits(2,1e-4,0.2);
	dgfit->SetParameter(3,param3);//SetParLimits(3,1.0,1e3);
	dgfit->SetParameter(4,hist->GetMean());
	dgfit->SetParLimits(5,0.,0.2);	
	dgfit->SetParNames("Constant 1","Mean 1","Sigma 1","Constant 2","Mean 2","Sigma 2");

	TFitResultPtr r1 = hist->Fit("dgfit", "RLMSIN");
	double sigma1 = dgfit->GetParameter(2);
	double sigma2 = dgfit->GetParameter(5);

	// extract full width at half max
	double max = hist->GetMaximum();
	int max_bin = hist->GetMaximumBin();
	double x_maximum = hist->GetXaxis()->GetBinCenter(max_bin);
	double half_max_height = max/2;

	// find x position of left side of FWHM
	double fwhm_left = 0;
	for (int ibin=0; ibin<int(hist->GetNbinsX()/2); ibin++) {
		double binheight = hist->GetBinContent(ibin);
		double binheight_next = hist->GetBinContent(ibin+1);
		if (binheight <= half_max_height && binheight_next >= half_max_height) { //
			double bincenter = hist->GetBinCenter(binheight);
			double bincenter_next = hist->GetBinCenter(binheight_next);
			fwhm_left = (bincenter+bincenter_next)/2;
		}
	}

	// find x position of right side of FWHM
	double fwhm_right = 0;
	for (int ibin=hist->GetNbinsX(); ibin>int(hist->GetNbinsX()/2); ibin--) {
		double binheight = hist->GetBinContent(ibin);
		double binheight_next = hist->GetBinContent(ibin-1);
		if (binheight <= half_max_height && binheight_next >= half_max_height) { //
			double bincenter = hist->GetBinCenter(binheight);
			double bincenter_next = hist->GetBinCenter(binheight_next);
			fwhm_right = (bincenter+bincenter_next)/2;
		}
	}

	double FWHM = fwhm_right - fwhm_left;

	// double max = dgfit->GetMaximum();
	// double x_maximum = dgfit->GetMaximumX();
	// double fwhm_left = dgfit->GetX(max/2,-0.2, x_maximum);
	// double fwhm_right = dgfit->GetX(max/2, x_maximum, 0.2);
	// double FWHM = fwhm_right - fwhm_left;
	const char* fwhm = to_string(FWHM).c_str();
	TPaveText* Pt2 = makeTexLabel(Form("%s%.4f", "FWHM = ", FWHM), 0.55, 0.65, 0.8, 0.8, 8);
	// makeTexLabel(Form("FWHM = %.4f", fwhm_right - fwhm_left), 0.6, 0.62, 0.8, 0.72, 8)->Draw();
	//Pt2->Draw();


	// double widths[3] = {sigma1, sigma2, FWHM};
	fit_widths[i][0] = sigma1;
	fit_widths[i][1] = sigma2;
	fit_widths[i][2] = FWHM;
	cout << "WIDTHS: " << fit_widths[i][0] << ", " << fit_widths[i][1] << ", " << fit_widths[i][2] << endl;


	return fit_widths; 
}



//Calculate full width at half max from standard deviation
double calcFullWidth(double st_dev){
	double width = 2 * sqrt(2 * log(2)) * st_dev;
	return width;
}




// adjust label size
void adjustLabelSize(TH1* hist, double size){
	hist->GetXaxis()->SetLabelSize(size);
	hist->GetYaxis()->SetLabelSize(size);
	hist->GetXaxis()->SetTitleSize(size);
	hist->GetYaxis()->SetTitleSize(size);
}








// ========================================================================================
// Main function
void plot_hists(TString input_fname = "eicrecon_plugin_brycecanyon_etarange_flat_thresh5keV.root", 
				TString output_fname = "plot_hists_brycecanyon_etarange_flat_thresh5keV.pdf"){
	// Variables that the user should specify
	input_fname = "eicrecon.root"; 
	output_fname = "plot_hists_brycecanyon_realistic_seeding.pdf"; //"plot_hists_etarange_flat.pdf";

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
	TH1 *hchi2_by_meas = (TH1*) f->Get("track_qa/hchi2_by_meas");

	TH2 *hchi2_vs_eta = (TH2*) f->Get("track_qa/hchi2_vs_eta");
	TH2 *hchi2NDF_vs_eta = (TH2*) f->Get("track_qa/hchi2NDF_vs_eta");
	TH2 *hchi2_vs_hits = (TH2*) f->Get("track_qa/hchi2_vs_hits");
	TH2 *hchi2_vs_hits_zoomed = (TH2*) f->Get("track_qa/hchi2_vs_hits_zoomed");
	// vector<TH2*> hchi2_vs_hits_etabins = (vector<TH2*>) f->Get("track_qa/hchi2_vs_hits_etabins");
	TH2 *hhits_vs_eta = (TH2*) f->Get("track_qa/hhits_vs_eta");
	TH2 *hhits_vs_eta_1 = (TH2*) f->Get("track_qa/hhits_vs_eta_1");
	TH2 *htracks_vs_eta = (TH2*) f->Get("track_qa/htracks_vs_eta");
	TH3 *heta_vs_p_vs_chi2 = (TH3*) f->Get("track_qa/heta_vs_p_vs_chi2");
	
	TH2 *hNDF_states = (TH2*) f->Get("track_qa/hNDF_states");

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
	TH2 *hsummation2 = (TH2*) f->Get("track_qa/hsummation2");
	TH2 *hsummation3 = (TH2*) f->Get("track_qa/hsummation3");

	TH2 *hmeaschi2_vs_volID = (TH2*) f->Get("track_qa/hmeaschi2_vs_volID");
	TH2 *hmeaschi2_vs_layID = (TH2*) f->Get("track_qa/hmeaschi2_vs_layID");
	TH2 *hmeaschi2_vs_vollayIDs = (TH2*) f->Get("track_qa/hmeaschi2_vs_vollayIDs");
	


	vector<TH1*> hhits_in_r(3);
	vector<TH1*> hhits_in_z(3);
	vector<TH2*> hhits_r_vs_z(3);
	vector<TH1*> hmeas_in_r(3);
	vector<TH1*> hmeas_in_z(3);
	vector<TH2*> hmeas_r_vs_z(3);
	vector<TH2*> hmeas_outliers_r_vs_z(3);
	vector<TH2*> hmeas_holes_r_vs_z(3);
	vector<TH1*> hresiduals(3);
	char eta_regions[3][20] = {"backward", "barrel", "forward"};
	vector<TPaveText*> tex_eta_regions(3);
	for (int i=0; i<3; i++){
		hhits_in_r[i] = (TH1*) f->Get(Form("track_qa/residuals/hhits_in_r_%s", eta_regions[i]));
		hhits_in_z[i] = (TH1*) f->Get(Form("track_qa/residuals/hhits_in_z_%s", eta_regions[i]));
		hhits_r_vs_z[i] = (TH2*) f->Get(Form("track_qa/residuals/hhits_r_vs_z_%s", eta_regions[i]));

		hmeas_in_r[i] = (TH1*) f->Get(Form("track_qa/residuals/hmeas_in_r_%s", eta_regions[i]));
		hmeas_in_z[i] = (TH1*) f->Get(Form("track_qa/residuals/hmeas_in_z_%s", eta_regions[i]));
		hmeas_r_vs_z[i] = (TH2*) f->Get(Form("track_qa/residuals/hmeas_r_vs_z_%s", eta_regions[i]));
		hmeas_outliers_r_vs_z[i] = (TH2*) f->Get(Form("track_qa/residuals/hmeas_outliers_r_vs_z_%s", eta_regions[i]));
		hmeas_holes_r_vs_z[i] = (TH2*) f->Get(Form("track_qa/residuals/hmeas_holes_r_vs_z_%s", eta_regions[i]));

		hresiduals[i] = (TH1*) f->Get(Form("track_qa/residuals/hresiduals_%s", eta_regions[i]));
		cout << "residual entries: " << hresiduals[i]->GetEntries()<<endl;
		TString text = Form("%s", eta_regions[i]);
		if (i==0) { //backward
			tex_eta_regions[i] = makeTexLabel(text, 0.55, 0.7, 0.7, 0.85, 10);
		} else {
			tex_eta_regions[i] = makeTexLabel(text, 0.5, 0.7, 0.7, 0.85, 10);
		}

		// tex_eta_regions[i] = new TPaveText(0.5,0.7,0.7,0.85,"NDCNB");
		// tex_eta_regions[i]->AddText(Form("%s", eta_regions[i]));
		// tex_eta_regions[i]->SetFillStyle(4000);tex_eta_regions[i]->SetTextFont(63);
		// tex_eta_regions[i]->SetTextSize(10);tex_eta_regions[i]->SetTextColor(kBlack);
	}


	TH1 *hvolID = (TH1*) f->Get("track_qa/hvolID");
	TH1 *hlayID = (TH1*) f->Get("track_qa/hlayID");
	TH1 *hvollayIDs = (TH1*) f->Get("track_qa/hvollayIDs");

	// vector<TH2*> htrackstate_r_vs_vollayIDs(20);
	// vector<TH2*> htrackstate_z_vs_vollayIDs(20);
	vector<TH1*> htrackstate_r(20);
	vector<TH1*> htrackstate_z(20);
	vector<TH2*> htrackstate_r_vs_z(20);
	vector<TH1*> hresiduals_vollaybins(20);
	// int vollay_arr[20] = {0,22,142,144,146,192,242,262,264,266,282,302,312,332,342,344,346,352,362,382}; //indices of all (volID*10 + layID) indices - ARCHES
	//int vollay_arr[20] = {0,22,122,124,126,172,222,242,244,246,262,282,292,312,322,324,326,332,342,362}; //indices of all (volID*10 + layID) indices - BRYCE CANYON
	int vollay_arr[20] = {0, 22, 132, 134, 136, 182, 232, 252, 254, 256, 272, 292, 302, 322, 332, 334, 336, 342, 352, 372}; //Realistic seeding
	char vollay_identities[20][20] = {"all","dead","b disk 5","b disk 4","b disk 3","b disk 2","b disk 1",
                                    "vertex 1","vertex 2","vertex 3","f disk 1","barrel sagitta 1","f disk 2",
                                    "barrel sagitta 2","f disk 3","f disk 4","f disk 5",
                                    "MPGD barrel","TOF barrel","TOF endcap"}; //"MPGD DIRC"}; //corresponding labels of all (volID*10 + layID) indices
    vector<TPaveText*> tex_vollay(20);
	for (int i=0; i<20; i++){
		// htrackstate_r_vs_vollayIDs[i] = (TH2*) f->Get(Form("track_qa/residuals/htrackstate_r_vs_vollayIDs_%d", vollay_arr[i]));
		// htrackstate_z_vs_vollayIDs[i] = (TH2*) f->Get(Form("track_qa/residuals/htrackstate_z_vs_vollayIDs_%d", vollay_arr[i]));
		htrackstate_r[i] = (TH1*) f->Get(Form("track_qa/residuals/htrackstate_r_%d", vollay_arr[i]));
		htrackstate_z[i] = (TH1*) f->Get(Form("track_qa/residuals/htrackstate_z_%d", vollay_arr[i]));
		htrackstate_r_vs_z[i] = (TH2*) f->Get(Form("track_qa/residuals/htrackstate_r_vs_z_%d", vollay_arr[i]));
		hresiduals_vollaybins[i] = (TH1*) f->Get(Form("track_qa/residuals/hresiduals_vollaybins_%d", vollay_arr[i]));
		printf("%d\n", hresiduals_vollaybins[i]);

		int layID = vollay_arr[i] % 10;
		int volID = floor(vollay_arr[i] / 10);
		tex_vollay[i] = makeTexLabel(Form("%s", vollay_identities[i]), 0.6, 0.75, 0.7, 0.9, 8);
		// tex_vollay[i]->AddText(Form("Volume ID: %d, Layer ID: %d", volID, layID));
	}

	// -----------------------------------------------

	auto fdiagline = new TF1("fdiagline","x",0,10);
	fdiagline->SetLineWidth(3);fdiagline->SetLineColor(kRed);
	auto fdiagline_thin = new TF1("fdiagline_thin","x",0,10);
	fdiagline_thin->SetLineWidth(2);fdiagline_thin->SetLineColor(kRed);

	// auto fvertline = new TF1("fvertline","x=0",-1000,1000);
	// fvertline->SetLineWidth(3);fvertline->SetLineColor(kRed);
	auto fvertline = new TLine(0,-1000,0,1000);
	fvertline->SetLineWidth(1);fvertline->SetLineColor(45);fvertline->SetLineStyle(2);
	auto fhoriline = new TF1("fhoriline","y=0",-2000,2000);
	fhoriline->SetLineWidth(1);fhoriline->SetLineColor(45);fhoriline->SetLineStyle(2);

	// -----------------------------------------------
	//Make plots
	const int nCanvas = 41;
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
	c1[2]->Divide(3,2);
	c1[2]->cd(1);
	hhits->Draw();
	c1[2]->cd(2);
	hNDF->Draw();
	c1[2]->cd(4);
	hchi2_by_hits->Draw();
	c1[2]->cd(5);
	hchi2_by_NDF->Draw();
	c1[2]->cd(6);
	hchi2_by_meas->Draw();
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
	//hchi2_vs_eta->Draw("colz");
	hchi2NDF_vs_eta->Draw("colz");

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
    hhits_vs_eta_1->Draw("colz");

	c1[10] = new TCanvas("c10","c10");
	htracks_vs_eta->Draw("colz");

	c1[11] = new TCanvas("c11","c11");
	heta_vs_p_vs_chi2->Draw("lego2");

	c1[12] = new TCanvas("c12","c12");
	hmeasptrack_vs_eta->Draw("colz");

	c1[13] = new TCanvas("c13","c13");
	hmeasptrack_vs_hits->Draw("colz");
	fdiagline->Draw("same");

	c1[14] = new TCanvas("c14","c14");
	c1[14]->Divide(4,4);
	for (int i=0; i<num_eta_bin; i++){
		c1[14]->cd(i+1);
		hmeasptrack_vs_hits_etabins[i]->Draw("colz");
		tex_eta[i]->Draw();
		fdiagline_thin->Draw("same");
	}

	c1[15] = new TCanvas("c15","c15");
	c1[15]->Divide(4,4);
	for (int i=0; i<num_eta_bin; i++){
		c1[15]->cd(i+1);
		hmeasptrack_vs_hits_etabins_zoomed[i]->Draw("colz");
		tex_eta[i]->Draw();
		fdiagline_thin->Draw("same");
	}

	c1[16] = new TCanvas("c16","c16");
	hmeasptrack_vs_chi2perNDF->Draw("colz");

	c1[17] = new TCanvas("c17","c17");
	c1[17]->Divide(4,4);
	for (int i=0; i<num_eta_bin; i++){
		c1[17]->cd(i+1);
		hmeasptrack_vs_chi2perNDF_etabins[i]->Draw("colz");
		tex_eta[i]->Draw();
	}

	c1[18] = new TCanvas("c18","c18");
	hmeasptrack_vs_calstates->Draw("colz");

	c1[19] = new TCanvas("c19","c19");
	hmeaschi2_vs_chi2->Draw("colz");

	c1[20] = new TCanvas("c20","c20");
	hmeaschi2_vs_eta->Draw("colz");

	c1[21] = new TCanvas("c21","c21");
	hmeaschi2_vs_hits->Draw("colz");

	c1[22] = new TCanvas("c22","c22");
	hholes_vs_hits->Draw("colz");

	c1[23] = new TCanvas("c23","c23");
	houtliers_vs_hits->Draw("colz");

	c1[24] = new TCanvas("c24","c24");
	hsummation2->Draw("colz");

	c1[25] = new TCanvas("c25","c25");
	hsummation3->Draw("colz");

	c1[26] = new TCanvas("c26","c26");
	hNDF_states->Draw("colz");

	c1[27] = new TCanvas("c27","c27");
	c1[27]->Divide(3,2);
	for (int i=0; i<3;i++){
		c1[27]->cd(i+1); //1, 2, 3
		hhits_in_r[i]->Draw();
		tex_eta_regions[i]->Draw();
		c1[27]->cd(i+4); //4, 5, 6
		hhits_in_z[i]->Draw();
		tex_eta_regions[i]->Draw();
	}

	c1[28] = new TCanvas("c28","c28");
	c1[28]->Divide(3,2);
	for (int i=0; i<3;i++){
		c1[28]->cd(i+1); //1, 2, 3
		hhits_in_r[i]->Draw();
		hmeas_in_r[i]->Draw("same");
		tex_eta_regions[i]->Draw();
		c1[28]->cd(i+4); //4, 5, 6
		hhits_in_z[i]->Draw();
		hmeas_in_z[i]->Draw("same");
		tex_eta_regions[i]->Draw();
	}

	c1[29] = new TCanvas("c29","c29");
	c1[29]->Divide(3,2);
	for (int i=0; i<3;i++){
		c1[29]->cd(i+1); //1, 2, 3
		hhits_r_vs_z[i]->Draw("colz");
		fvertline->Draw("same");
		fhoriline->Draw("same");
		tex_eta_regions[i]->Draw();
		c1[29]->cd(i+4); //4, 5, 6
		hmeas_r_vs_z[i]->Draw("colz");
		fvertline->Draw("same");
		fhoriline->Draw("same");
		tex_eta_regions[i]->Draw();
	}

	c1[30] = new TCanvas("c30","c30");
	hmeaschi2_vs_volID->Draw("colz");

	c1[31] = new TCanvas("c31","c31");
	hmeaschi2_vs_layID->Draw("colz");

	c1[32] = new TCanvas("c32","c32");
	hmeaschi2_vs_vollayIDs->Draw("colz");

	c1[33] = new TCanvas("c33","c33");
	c1[33]->Divide(3,2);
	for (int i=0; i<3;i++){
		c1[33]->cd(i+1); //1, 2, 3
		hresiduals[i]->Draw();
		// Double_t stddev = makeFits(hresiduals[i]); double fullwidth = calcFullWidth(stddev);
		// gStyle->SetOptFit(1111);
		// gStyle->SetStatY(0.5);
		// gStyle->SetStatFontSize(12);
		TF1 *dgfit = new TF1("dgfit","gaus(0)+gaus(3)",-0.1,0.1);
		dgfit->SetParameter(0,800);//SetParLimits(0,1.0,1e3);
		dgfit->SetParameter(3,2000);//SetParLimits(3,1.0,1e3);
		dgfit->SetParameter(1,hresiduals[i]->GetMean());
		dgfit->SetParameter(4,hresiduals[i]->GetMean());
		dgfit->SetParLimits(2,1e-4,0.2);
		dgfit->SetParLimits(5,0.,0.2);
	    dgfit->SetParNames("Constant 1","Mean 1","Sigma 1","Constant 2","Mean 2","Sigma 2");

		//TFitResultPtr r1 = hresiduals[i]->Fit("dgfit", "RLMSI"); // option "N" is do not draw
		TFitResultPtr r1 = hresiduals[i]->Fit("dgfit", "RLMSIN"); // option "N" is do not draw
		double const1 = r1->Parameter(0);
		double mean1 = r1->Parameter(1);
		double sigma1 = r1->Parameter(2);
		double const2 = r1->Parameter(3);
		double mean2 = r1->Parameter(4);
		double sigma2 = r1->Parameter(5);

		tex_eta_regions[i]->Draw();

		double max = dgfit->GetMaximum();
		double x_maximum = dgfit->GetMaximumX();
		double fwhm_left = dgfit->GetX(max/2,-0.2, x_maximum);
		double fwhm_right = dgfit->GetX(max/2, x_maximum, 0.2);
		const char* fwhm = to_string(fwhm_right - fwhm_left).c_str();
		TPaveText* pt = makeTexLabel(Form("%s%s", "FWHM = ", fwhm), 0.0, 0.75, 0.7, 0.9, 8);
		// makeTexLabel(Form("FWHM = %.4f", fwhm_right - fwhm_left), 0.6, 0.62, 0.8, 0.72, 8)->Draw();
		pt->Draw();

		c1[33]->cd(i+4); //4, 5, 6
		gPad->SetLogy();
		hresiduals[i]->Draw();
		// gStyle->SetOptFit(1111);
		// gStyle->SetStatY(0.5);
		// gStyle->SetStatFontSize(12);
		tex_eta_regions[i]->Draw();
		// makeTexLabel(Form("Width = %.4f", fullwidth), 0.6, 0.62, 0.8, 0.72, 8)->Draw();
	}

	c1[34] = new TCanvas("c34","c34");
	c1[34]->Divide(2,2);
	c1[34]->cd(1);
	hvolID->Draw();
	c1[34]->cd(2);
	hlayID->Draw();
	c1[34]->cd(3);
	hvollayIDs->Draw();

	c1[35] = new TCanvas("c35","c35");
	c1[35]->Divide(4,5);
	for (int i=0; i<20;i++){
		c1[35]->cd(i+1);
		// htrackstate_r_vs_vollayIDs[i]->Draw("colz");
		htrackstate_r[i]->Draw("colz");
		tex_vollay[i]->Draw();
	}

	c1[36] = new TCanvas("c36","c36");
	c1[36]->Divide(4,5);
	for (int i=0; i<20;i++){
		c1[36]->cd(i+1);
		// htrackstate_z_vs_vollayIDs[i]->Draw("colz");
		htrackstate_z[i]->Draw("colz");
		tex_vollay[i]->Draw();
	}

	c1[37] = new TCanvas("c37","c37");
	c1[37]->Divide(4,5);
	for (int i=0; i<20;i++){
		c1[37]->cd(i+1);
		htrackstate_r_vs_z[i]->Draw("colz");
		// if (i != 9 || i!= 17) fvertline->Draw("same");
		// fhoriline->Draw("same");
		tex_vollay[i]->Draw();
	}

	c1[38] = new TCanvas("c38","c38");
	c1[38]->Divide(3,2);
	for (int i=0; i<3;i++){
		c1[38]->cd(i+1); //1, 2, 3
		hmeas_outliers_r_vs_z[i]->Draw("colz");
		tex_eta_regions[i]->Draw();
		c1[38]->cd(i+4); //4, 5, 6
		hmeas_holes_r_vs_z[i]->Draw("colz");
		tex_eta_regions[i]->Draw();
	}

	c1[39] = new TCanvas("c39","c39");
	c1[39]->Divide(4,5);
	// adjustLabelSize(4,5);

	double** fit_widths = 0;
	fit_widths = new double*[18];
	double c1_params[nCanvas] = {30,-1,-1,40,-1, 1000,10,60,-1,-1, 40,40,-1,-1,20, -1,-1,-1};
	double c2_params[nCanvas] = {130,-1,-1,80,-1, 1000,1000,100,-1,-1, 100,150,-1,-1,80, -1,-1,-1};

	for (int i=0; i<18;i++){
		c1[39]->cd(i+1);
		hresiduals_vollaybins[i+2]->Draw();
		adjustLabelSize(hresiduals_vollaybins[i+2], 0.07);
		// Double_t stddev = makeFits(hresiduals_vollaybins[i+2]);
		

		fit_widths[i] = new double[3];
		if (c1_params[i] == -1){
			fit_widths = makeDGFits(hresiduals_vollaybins[i+2], fit_widths, i);
		} else {
			fit_widths = makeDGFits(hresiduals_vollaybins[i+2], fit_widths, i, c1_params[i], c2_params[i]);
		}
		// gStyle->SetOptFit(1111);
		// gStyle->SetStatY(0.7);

		// TFitResultPtr r1 = hresiduals_vollaybins[i+2]->Fit("dgfit", "RLMSIN"); // option "N" is do not draw
		// double const1 = r1->Parameter(0);
		// double mean1 = r1->Parameter(1);
		// double sigma1 = r1->Parameter(2);
		// double const2 = r1->Parameter(3);
		// double mean2 = r1->Parameter(4);
		// double sigma2 = r1->Parameter(5);
		tex_vollay[i+2]->Draw();
	}

	c1[40] = new TCanvas("c40","c40");
	c1[40]->Divide(4,5);
	// c1[40]->SetLogy();
	for (int i=0; i<18;i++){
		c1[40]->cd(i+1);
		gPad->SetLogy();
		hresiduals_vollaybins[i+2]->Draw();
		// Double_t stddev = makeFits(hresiduals_vollaybins[i+2]);
		gStyle->SetOptFit(1111);
		gStyle->SetStatY(0.7);
		tex_vollay[i+2]->Draw();
	}
	


	for(int i = 0 ; i < nCanvas ; i++){
		if(i==0) c1[i]->Print(output_fname+"(");
		else if (i==nCanvas-1) c1[i]->Print(output_fname+")");
		else c1[i]->Print(output_fname);
	}

	
}
