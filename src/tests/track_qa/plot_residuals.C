#include <iostream>
#include <vector>
using namespace std;


// ========================================================================================
// Make gaussian fits
Double_t makeGaussianFits(TH1* hist){ //(vector<TH1F*> hists, Double_t* st_dev=NULL){

    double lowerbound = -0.5;
    double upperbound = 0.5;

    Double_t st_dev = 0;
	TF1* g1 = new TF1("g1", "gaus",lowerbound,upperbound);

	hist->Fit(g1, "RQ");
	st_dev = g1->GetParameter(2);

    return st_dev;
}


double*** makeDGFits(TH1* hist, double*** fit_widths, int i, int j, double param0=10., double param3=50.){
	TF1 *dgfit = new TF1("dgfit","gaus(0)+gaus(3)",-0.1,0.1);

	dgfit->SetParameter(0,param0);//SetParLimits(0,1.0,1e3);
	dgfit->SetParameter(1,hist->GetMean());
	dgfit->SetParLimits(2,1e-4,0.2);
	dgfit->SetParameter(3,param3);//SetParLimits(3,1.0,1e3);
	dgfit->SetParameter(4,hist->GetMean());
	dgfit->SetParLimits(5,0.,0.2);	
	dgfit->SetParNames("Constant 1","Mean 1","Sigma 1","Constant 2","Mean 2","Sigma 2");

	TFitResultPtr r1 = hist->Fit("dgfit", "RLMSI");
	double sigma1 = dgfit->GetParameter(2);
	double sigma2 = dgfit->GetParameter(5);

	// extract full width at half max
	double max = dgfit->GetMaximum();
	double x_maximum = dgfit->GetMaximumX();
	double fwhm_left = dgfit->GetX(max/2,-0.2, x_maximum);
	double fwhm_right = dgfit->GetX(max/2, x_maximum, 0.2);
	double FWHM = fwhm_right - fwhm_left;

	// double widths[3] = {sigma1, sigma2, FWHM};
	fit_widths[i][j][0] = sigma1;
	fit_widths[i][j][1] = sigma2;
	fit_widths[i][j][2] = FWHM;
	cout << "WIDTHS: " << fit_widths[i][j][0] << ", " << fit_widths[i][j][1] << ", " << fit_widths[i][j][2] << endl;


	return fit_widths; 
}




//Calculate full width at half max from standard deviation
double calcFullWidth(double st_dev){
	double width = 2 * sqrt(2 * log(2)) * st_dev;
	return width;
}


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


// adjust label size
void adjustLabelSize(TH1* hist, double size){
	hist->GetXaxis()->SetLabelSize(size);
	hist->GetYaxis()->SetLabelSize(size);
	hist->GetXaxis()->SetTitleSize(size);
	hist->GetYaxis()->SetTitleSize(size);
}




// ========================================================================================
// Plot the widths

void plot_widths(double*** fit_widths){
	const int nlayers = 18;
	const int npoints = 4;
	double pbins[4] = {2,5,7,10};

	// const int nCanvas = 18;
	// TCanvas ** c2 = new TCanvas * [nCanvas];

	// for (int i=0; i<nCanvas; i++){
	// 	c2[i] = new TCanvas(Form("c%d",i),"");
	// 	c2[i]->Divide(2,2);




	vector<TGraph*> graph_largewidths(18);
	vector<TGraph*> graph_smallwidths(18);
	vector<TGraph*> graph_calcwidths(18);
	TCanvas *c2_graph = new TCanvas("c2_graph","c2_graph");
	c2_graph->Divide(5,4);

	for (int i=0; i<nlayers; i++){
		//extract proper arrays from fit_widths
		double largewidths[npoints]; double smallwidths[npoints]; double calcwidths[npoints];
		for (int j=0; j<npoints; j++){
			largewidths[j] = fit_widths[i][j][0];
			smallwidths[j] = fit_widths[i][j][1];
			calcwidths[j] = fit_widths[i][j][2];
		}

		//make TGraphs of all the widths
		graph_largewidths[i] = new TGraph(npoints,pbins,largewidths);
		graph_smallwidths[i] = new TGraph(npoints,pbins,smallwidths);
		graph_calcwidths[i] = new TGraph(npoints,pbins,calcwidths);

		graph_largewidths[i]->SetTitle("Width of Double Gaussian Fits; p [GeV/c]; Width [mm]");

		c2_graph->cd(i+1);

		graph_largewidths[i]->SetMarkerColor(2);
		graph_smallwidths[i]->SetMarkerColor(4);
		graph_calcwidths[i]->SetMarkerColor(6);

		graph_largewidths[i]->Draw("A*");
		graph_smallwidths[i]->Draw("A* same");
		// graph_calcwidths[i]->Draw("A*SAME");

	}

	c2_graph->Print("plot_widths_brycecanyon_etarange_flat_thresh5keV.pdf");
   	


}


void write_widths(double*** fit_widths){

	//write width information to file
	char newfilename[24] = "widths.dat";

	ofstream outdata; // outdata is like cin
	outdata.open(newfilename); // opens the file
	if( !outdata ) { // file couldn't be opened
		cerr << "Error: file could not be opened" << endl;
		exit(1);
	}

	int nlayers = 18; //TODO: make automastic
	int npbins = 4; //TODO: make automastic
	cout << "nlayers " << nlayers << ", " << npbins << endl;
	for (int i=0; i<nlayers; i++){
		for (int j=0; j<npbins; j++){
			outdata << i << " " << j << " " 
					<< fit_widths[i][j][0] << " " << fit_widths[i][j][1] 
										   << " " << fit_widths[i][j][2] << endl;
		}
	}
		
	outdata.close();

}





// ========================================================================================
// Main function
void plot_residuals(TString input_fname="eicrecon_plugin_brycecanyon_etarange_flat_thresh5keV.root", 
				    TString output_fname="plot_residuals_brycecanyon_etarange_flat_thresh5keV.pdf"){
	// Variables that the user should specify
	// TString input_fname = "eicrecon_plugin_brycecanyon_etarange_flat_thresh5keV.root"; //"eicrecon.root"; //"eicrecon_plugin_etarange_flat.root"; 
	// TString output_fname = "plot_residuals_brycecanyon_etarange_flat_thresh5keV.pdf"; //"plot_hists_etarange_flat.pdf";

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

	
	vector<TH1*> hresiduals(3);
	char eta_regions[3][20] = {"backward", "barrel", "forward"};
	vector<TPaveText*> tex_eta_regions(3);
	for (int i=0; i<3; i++){
		hresiduals[i] = (TH1*) f->Get(Form("track_qa/residuals/hresiduals_%s", eta_regions[i]));

		TString text = Form("%s", eta_regions[i]);
		if (i==0) { //backward
			tex_eta_regions[i] = makeTexLabel(text, 0.55, 0.7, 0.7, 0.85, 10);
		} else {
			tex_eta_regions[i] = makeTexLabel(text, 0.5, 0.7, 0.7, 0.85, 10);
		}
	}


	vector<TH1*> hresiduals_vollaybins(20);
	// int vollay_arr[20] = {0,22,142,144,146,192,242,262,264,266,282,302,312,332,342,344,346,352,362,382}; //indices of all (volID*10 + layID) indices - ARCHES
	int vollay_arr[20] = {0,22,122,124,126,172,222,242,244,246,262,282,292,312,322,324,326,332,342,362}; //indices of all (volID*10 + layID) indices - BRYCE CANYON
	char vollay_identities[20][20] = {"all","dead","b disk 5","b disk 4","b disk 3","b disk 2","b disk 1",
                                    "vertex 1","vertex 2","vertex 3","f disk 1","barrel sagitta 1","f disk 2",
                                    "barrel sagitta 2","f disk 3","f disk 4","f disk 5",
                                    "MPGD barrel","TOF barrel","TOF endcap"}; //"MPGD DIRC"}; //corresponding labels of all (volID*10 + layID) indices
    vector<TPaveText*> tex_vollay(20);
	for (int i=0; i<20; i++){
		hresiduals_vollaybins[i] = (TH1*) f->Get(Form("track_qa/residuals/hresiduals_vollaybins_%d", vollay_arr[i]));

		// int layID = vollay_arr[i] % 10;
		// int volID = floor(vollay_arr[i] / 10);
		tex_vollay[i] = makeTexLabel(Form("%s", vollay_identities[i]), 0.5, 0.75, 0.7, 0.9, 8);
	}
// TString::Format("hresiduals_vollay_%d_mom_%s", vollay_arr[i], mom_bins_arr[j])

	vector<vector<TH1*>> hresiduals_layers_in_pbins;
	char mom_bins_arr[4][20] = {"1-2","2-5","5-7","7-10"};
	char mom_bins_labels[4][20] = {"1-2 GeV","2-5 GeV","5-7 GeV","7-10 GeV"};
	for (int i=0; i<20; i++){
		vector<TH1*> v1;
		// hresiduals_layers_in_pbins.push_back(vector<TH1*>());
		for (int j=0; j<4; j++){
			TH1* h1 = (TH1*) f->Get(Form("track_qa/residuals/hresiduals_vollay_%d_mom_%s", vollay_arr[i], mom_bins_arr[j]));
			v1.push_back(h1);
		}
		hresiduals_layers_in_pbins.push_back(v1);
	}

	// for (int i=0; i<hresiduals_layers_in_pbins.size(); i++){
	// 	for (int j=0; j<hresiduals_layers_in_pbins[i].size(); j++){
	// 		cout << "hi " << i << ", " << j << endl;
	// 	}
	// }



	// -----------------------------------------------
	//Make plots
	const int nCanvas = 18;
	TCanvas ** c1 = new TCanvas * [nCanvas];

	double c1_params[nCanvas][4] = {{-1,-1,-1,20},{-1,-1,-1,-1},{-1,10,10,40},{-1,10,10,-1},{-1,10,10,-1},
								{10,20,100,100},{-1,50,50,100},{5,20,10,16},{1,10,10,20},{4,10,-1,-1},
								{-1,8,5,20},{-1,-1,-1,20},{2,10,10,10},{2,10,10,-1},{-1,10,10,3},
								{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}};
	double c2_params[nCanvas][4] = {{-1,-1,-1,85},{-1,-1,-1,-1},{-1,30,5,60},{-1,30,30,-1},{-1,20,25,-1},
								{130,520,300,600},{-1,300,200,350},{15,40,30,57},{5,20,22,55},{12,60,-1,-1},
								{-1,20,25,45},{-1,-1,-1,90},{5,30,20,60},{5,25,10,-1},{-1,20,15,50},
								{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}};

	double*** fit_widths = 0;
	fit_widths = new double**[18];

	for (int i=0; i<nCanvas; i++){
		c1[i] = new TCanvas(Form("c%d",i),"");
		c1[i]->Divide(2,2);

		fit_widths[i] = new double*[4];

		for (int j=0; j<4; j++){
			c1[i]->cd(j+1);
			hresiduals_layers_in_pbins[i+2][j]->Draw();	
			makeTexLabel(Form("%s, %s", vollay_identities[i+2], mom_bins_labels[j]), 0.15, 0.75, 0.35, 0.9, 8)->Draw();
			// makeTexLabel(mom_bins_labels[j], 0.25, 0.72, 0.45, 0.82, 8)->Draw();
			// need to add momentum labels to the plot

			// fit
			// makeGaussianFits(hresiduals_layers_in_pbins[i+2][j]);
			// TFitResultPtr r1 = makeDGFits(hresiduals_layers_in_pbins[i+2][j]);
			// double* fit_widths;
			fit_widths[i][j] = new double[3];
			if (c1_params[i][j] == -1){
				fit_widths = makeDGFits(hresiduals_layers_in_pbins[i+2][j], fit_widths, i, j);
			} else {
				fit_widths = makeDGFits(hresiduals_layers_in_pbins[i+2][j], fit_widths, i, j, c1_params[i][j], c2_params[i][j]);
			}
			gStyle->SetOptFit(1111);

			makeTexLabel(Form("FWHM = %.4f", fit_widths[i][j][2]), 0.15, 0.7, 0.35, 0.8, 8)->Draw();
			cout << "FIT WIDTHS: " << fit_widths[i][j][0] << ", " << fit_widths[i][j][1] << ", " << fit_widths[i][j][2] << endl;
		}
	}

	// plot_widths(fit_widths);
	write_widths(fit_widths);
	
	
	// Print to file
	for(int i = 0 ; i < nCanvas ; i++){
		if(i==0) c1[i]->Print(output_fname+"(");
		else if (i==nCanvas-1) c1[i]->Print(output_fname+")");
		else c1[i]->Print(output_fname);
	}

	
}

