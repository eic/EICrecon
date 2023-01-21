void plot_hists(){

    //Define Style
    gStyle->SetOptStat(0);
    gStyle->SetPadBorderMode(0);
    gStyle->SetFrameBorderMode(0);
    gStyle->SetFrameLineWidth(2);
    gStyle->SetLabelSize(0.035,"X");
    gStyle->SetLabelSize(0.035,"Y");
    //gStyle->SetLabelOffset(0.01,"X");
    //gStyle->SetLabelOffset(0.01,"Y");
    gStyle->SetTitleXSize(0.04);
    gStyle->SetTitleXOffset(0.9);
    gStyle->SetTitleYSize(0.04);
    gStyle->SetTitleYOffset(0.9);

    //Get ROOT file
    TFile *f = new TFile("eicrecon.root");
    TH2 *h1a = (TH2*) f->Get("Eemc_TrkPropagation/h1a");
    TH2 *h1b = (TH2*) f->Get("Eemc_TrkPropagation/h1b");
    TH1 *h2a = (TH1*) f->Get("Eemc_TrkPropagation/h2a");
    TH1 *h2b = (TH1*) f->Get("Eemc_TrkPropagation/h2b");
    TH1 *h3a = (TH2*) f->Get("Eemc_TrkPropagation/h3a");
    TH1 *h3b = (TH2*) f->Get("Eemc_TrkPropagation/h3b");
    TH1 *h4a = (TH1*) f->Get("Eemc_TrkPropagation/h4a");
    TH1 *h4b = (TH1*) f->Get("Eemc_TrkPropagation/h4b");
    TH1 *h5a = (TH1*) f->Get("Eemc_TrkPropagation/h5a");

    //Make plots
    TCanvas *c1a = new TCanvas("c1a");
    h1a->Draw("colz");

    TPaveText* tex_gen = new TPaveText(0.2,0.7,0.5,0.85,"NDCNB");
    tex_gen->AddText("Single Electrons generated:");
    tex_gen->AddText("1 GeV < E < 20 GeV");
    tex_gen->AddText("160^{o} < #theta < 170^{o}, 0^{o} < #phi < 360^{o}");
	tex_gen->SetFillStyle(4000);tex_gen->SetTextFont(63);tex_gen->SetTextSize(20);
    tex_gen->Draw();

    TCanvas *c1b = new TCanvas("c1b");
    h1b->Draw("colz");

    TCanvas *c2 = new TCanvas("c2");
    c2->Divide(2,1);
    c2->cd(1);h2a->Draw();
    c2->cd(2);h2b->Draw();

    TCanvas *c3 = new TCanvas("c3");
    c3->Divide(2,1);
    c3->cd(1);h3a->Draw();
    c3->cd(2);h3b->Draw();

    TCanvas *c4 = new TCanvas("c4");
    c4->Divide(2,1);
    c4->cd(1);h4a->Draw();
    c4->cd(2);h4b->Draw();

    TCanvas *c5 = new TCanvas("c5");
    c5->cd(1);h5a->Draw();

    //Print plots to file
    c1a->Print("plot_hists.pdf[");
    c1a->Print("plot_hists.pdf");
    c1b->Print("plot_hists.pdf");
    c2->Print("plot_hists.pdf");
    c3->Print("plot_hists.pdf");
    c4->Print("plot_hists.pdf");
    c5->Print("plot_hists.pdf");
    c5->Print("plot_hists.pdf]");

}