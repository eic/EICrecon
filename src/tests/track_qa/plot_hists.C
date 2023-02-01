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
    TH2 *h1a = (TH2*) f->Get("track_qa/h1a");

    //Make plots
    TCanvas *c1a = new TCanvas("c1a");
    h1a->Draw("colz");

    TPaveText* tex_gen = new TPaveText(0.2,0.7,0.5,0.85,"NDCNB");
    tex_gen->AddText("Single Electrons generated:");
    tex_gen->AddText("1 GeV < E < 10 GeV");
    tex_gen->AddText("#theta = 90^{o}, 0^{o} < #phi < 360^{o}");
    tex_gen->SetFillStyle(4000);tex_gen->SetTextFont(63);tex_gen->SetTextSize(20);
    tex_gen->Draw();

    //Print plots to file
    c1a->Print("plot_hists.pdf[");
    c1a->Print("plot_hists.pdf");
    c1a->Print("plot_hists.pdf]");
}