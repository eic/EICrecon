#include "trackqa_processor.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <Math/GenVector/PxPyPzM4D.h>

#include <spdlog/spdlog.h>

#include <algorithms/tracking/JugTrack/TrackingResultTrajectory.hpp>
#include <algorithms/tracking/ParticlesFromTrackFitResult.h>

#include <services/rootfile/RootFile_service.h>
#include <services/geometry/acts/ACTSGeo_service.h>

#include <Acts/EventData/MultiTrajectory.hpp>
#include <Acts/EventData/MultiTrajectoryHelpers.hpp>

#include <edm4hep/MCParticle.h>

#include "TVectorT.h"

//------------------
// (Constructor)
//------------------
trackqa_processor::trackqa_processor(JApplication *app) :
	JEventProcessor(app)
{
}

//------------------
// Init
//------------------
void trackqa_processor::Init()
{
    std::string plugin_name=("track_qa");

    // Get JANA application
    auto app = GetApplication();

    // Ask service locator a file to write histograms to
    auto root_file_service = app->GetService<RootFile_service>();

    // Get TDirectory for histograms root file
    auto globalRootLock = app->GetService<JGlobalRootLock>();
    globalRootLock->acquire_write_lock();
    auto file = root_file_service->GetHistFile();
    globalRootLock->release_lock();

    // Create a directory for this plugin. And subdirectories for series of histograms
    m_dir_main = file->mkdir(plugin_name.c_str());
    m_dir_sub = m_dir_main->mkdir("eta_bins");
    m_dir_res = m_dir_main->mkdir("residuals");
    //Define histograms
    h1a = new TH2D("h1a","",100,0,25,100,0,25);
    h1a->GetXaxis()->SetTitle("True Momentum [GeV/c]");h1a->GetXaxis()->CenterTitle();
    h1a->GetYaxis()->SetTitle("Rec. Track Momentum [GeV/c]");h1a->GetYaxis()->CenterTitle();
    h1a->SetDirectory(m_dir_main);

    hchi2 = new TH1D("hchi2","",50,0,50);
    hchi2->GetXaxis()->SetTitle("Track #Chi^{2} Sum");hchi2->GetXaxis()->CenterTitle();
    hchi2->GetYaxis()->SetTitle("Counts");hchi2->GetYaxis()->CenterTitle();
    hchi2->SetLineWidth(2);hchi2->SetLineColor(kBlue);
    hchi2->SetDirectory(m_dir_main);

    heta = new TH1D("heta","",50,-4,4);
    heta->GetXaxis()->SetTitle("#eta (Generated)");heta->GetXaxis()->CenterTitle();
    heta->GetYaxis()->SetTitle("Counts");heta->GetYaxis()->CenterTitle();
    heta->SetLineWidth(2);heta->SetLineColor(kBlue);
    heta->SetDirectory(m_dir_main);

    hp = new TH1D("hp","",44,0,11);
    hp->GetXaxis()->SetTitle("Momentum [GeV]");hp->GetXaxis()->CenterTitle();
    hp->GetYaxis()->SetTitle("Counts");hp->GetYaxis()->CenterTitle();
    hp->SetLineWidth(2);hp->SetLineColor(kBlue);
    hp->SetDirectory(m_dir_main);

    hpt = new TH1D("hpt","",50,0,10);
    hpt->GetXaxis()->SetTitle("Transverse Momentum [GeV]");hpt->GetXaxis()->CenterTitle();
    hpt->GetYaxis()->SetTitle("Counts");hpt->GetYaxis()->CenterTitle();
    hpt->SetLineWidth(2);hpt->SetLineColor(kBlue);
    hpt->SetDirectory(m_dir_main);

    hhits = new TH1D("hhits","",50,0,50);
    hhits->GetXaxis()->SetTitle("Number of Hits Found in Tracker");hhits->GetXaxis()->CenterTitle();
    hhits->GetYaxis()->SetTitle("Counts");hhits->GetYaxis()->CenterTitle();
    hhits->SetLineWidth(2);hhits->SetLineColor(kBlue);
    hhits->SetDirectory(m_dir_main);

    hNDF = new TH1D("hNDF","",25,0,25);
    hNDF->GetXaxis()->SetTitle("NDF");hNDF->GetXaxis()->CenterTitle();
    hNDF->GetYaxis()->SetTitle("Counts");hNDF->GetYaxis()->CenterTitle();
    hNDF->SetLineWidth(2);hNDF->SetLineColor(kBlue);
    hNDF->SetDirectory(m_dir_main);

    hchi2_by_hits = new TH1D("hchi2_by_hits","",50,0,10);
    hchi2_by_hits->GetXaxis()->SetTitle("Track #Chi^{2} Sum/Number of Hits");hchi2_by_hits->GetXaxis()->CenterTitle();
    hchi2_by_hits->GetYaxis()->SetTitle("Counts");hchi2_by_hits->GetYaxis()->CenterTitle();
    hchi2_by_hits->SetLineWidth(2);hchi2_by_hits->SetLineColor(kBlue);
    hchi2_by_hits->SetDirectory(m_dir_main);

    hchi2_by_NDF = new TH1D("hchi2_by_NDF","",50,0,10);
    hchi2_by_NDF->GetXaxis()->SetTitle("Track #Chi^{2} Sum/NDF");hchi2_by_NDF->GetXaxis()->CenterTitle();
    hchi2_by_NDF->GetYaxis()->SetTitle("Counts");hchi2_by_NDF->GetYaxis()->CenterTitle();
    hchi2_by_NDF->SetLineWidth(2);hchi2_by_NDF->SetLineColor(kBlue);
    hchi2_by_NDF->SetDirectory(m_dir_main);

    hchi2_by_meas = new TH1D("hchi2_by_meas","",50,0,10);
    hchi2_by_meas->GetXaxis()->SetTitle("Track #Chi^{2} Sum/meas");hchi2_by_meas->GetXaxis()->CenterTitle();
    hchi2_by_meas->GetYaxis()->SetTitle("Counts");hchi2_by_meas->GetYaxis()->CenterTitle();
    hchi2_by_meas->SetLineWidth(2);hchi2_by_meas->SetLineColor(kBlue);
    hchi2_by_meas->SetDirectory(m_dir_main);

    //chi^2 and number of hits
    hchi2_vs_eta = new TH2D("hchi2_vs_eta","",50,-4,4,50,0,50);
    hchi2_vs_eta->GetXaxis()->SetTitle("#eta (Generated)");hchi2_vs_eta->GetXaxis()->CenterTitle();
    hchi2_vs_eta->GetYaxis()->SetTitle("Track #Chi^{2} Sum");hchi2_vs_eta->GetYaxis()->CenterTitle();
    hchi2_vs_eta->SetDirectory(m_dir_main);

    hchi2_vs_hits = new TH2D("hchi2_vs_hits","",50,0,50,50,0,50);
    hchi2_vs_hits->GetXaxis()->SetTitle("Number of Hits");hchi2_vs_hits->GetXaxis()->CenterTitle();
    hchi2_vs_hits->GetYaxis()->SetTitle("Track #Chi^{2} Sum");hchi2_vs_hits->GetYaxis()->CenterTitle();
    hchi2_vs_hits->SetDirectory(m_dir_main);

    hchi2_vs_hits_zoomed = new TH2D("hchi2_vs_hits_zoomed","",10,0,10,50,0,5);
    hchi2_vs_hits_zoomed->GetXaxis()->SetTitle("Number of Hits");hchi2_vs_hits_zoomed->GetXaxis()->CenterTitle();
    hchi2_vs_hits_zoomed->GetYaxis()->SetTitle("Track #Chi^{2} Sum");hchi2_vs_hits_zoomed->GetYaxis()->CenterTitle();
    hchi2_vs_hits_zoomed->SetDirectory(m_dir_main);

    hhits_vs_eta = new TH2D("hhits_vs_eta","",50,-4,4,50,0,50);
    hhits_vs_eta->GetXaxis()->SetTitle("#eta (Generated)");hhits_vs_eta->GetXaxis()->CenterTitle();
    hhits_vs_eta->GetYaxis()->SetTitle("Number of Hits");hhits_vs_eta->GetYaxis()->CenterTitle();
    hhits_vs_eta->SetDirectory(m_dir_main);

    hhits_vs_eta_1 = new TH2D("hhits_vs_eta_1","At least one track reconstructed",50,-4,4,50,0,50);
    hhits_vs_eta_1->GetXaxis()->SetTitle("#eta (Generated)");hhits_vs_eta_1->GetXaxis()->CenterTitle();
    hhits_vs_eta_1->GetYaxis()->SetTitle("Number of Hits");hhits_vs_eta_1->GetYaxis()->CenterTitle();
    hhits_vs_eta_1->SetDirectory(m_dir_main);

    htracks_vs_eta = new TH2D("htracks_vs_eta","",50,-4,4,10,0,10);
    htracks_vs_eta->GetXaxis()->SetTitle("#eta (Generated)");htracks_vs_eta->GetXaxis()->CenterTitle();
    htracks_vs_eta->GetYaxis()->SetTitle("Number of Tracks");htracks_vs_eta->GetYaxis()->CenterTitle();
    htracks_vs_eta->SetDirectory(m_dir_main);

    heta_vs_p_vs_chi2 = new TH3D("heta_vs_p_vs_chi2","",50,-4,4,44,0,11,50,0,50); //x: eta, y: p, z: chi^2
    heta_vs_p_vs_chi2->GetXaxis()->SetTitle("#eta (Generated)"); heta_vs_p_vs_chi2->GetXaxis()->CenterTitle();
    heta_vs_p_vs_chi2->GetYaxis()->SetTitle("Momentum [GeV]"); heta_vs_p_vs_chi2->GetYaxis()->CenterTitle();
    heta_vs_p_vs_chi2->GetZaxis()->SetTitle("Track #Chi^{2} Sum"); heta_vs_p_vs_chi2->GetZaxis()->CenterTitle();
    heta_vs_p_vs_chi2->SetDirectory(m_dir_main);

    hNDF_states = new TH2D("hNDF_states","",25,0,25,25,0,25);
    hNDF_states->GetXaxis()->SetTitle("NDF");hNDF_states->GetXaxis()->CenterTitle();
    hNDF_states->GetYaxis()->SetTitle("States");hNDF_states->GetYaxis()->CenterTitle();
    hNDF_states->SetDirectory(m_dir_main);

    hmeasptrack_vs_eta = new TH2D("hmeasptrack_vs_eta","",50,-4,4,10,0,10);
    hmeasptrack_vs_eta->GetXaxis()->SetTitle("#eta (Generated)");hmeasptrack_vs_eta->GetXaxis()->CenterTitle();
    hmeasptrack_vs_eta->GetYaxis()->SetTitle("Number of Measurements per Track");hmeasptrack_vs_eta->GetYaxis()->CenterTitle();
    hmeasptrack_vs_eta->SetDirectory(m_dir_main);

    hmeasptrack_vs_hits = new TH2D("hmeasptrack_vs_hits","",50,0,50,10,0,10);
    hmeasptrack_vs_hits->GetXaxis()->SetTitle("Number of Hits");hmeasptrack_vs_hits->GetXaxis()->CenterTitle();
    hmeasptrack_vs_hits->GetYaxis()->SetTitle("Number of Measurements per Track");hmeasptrack_vs_hits->GetYaxis()->CenterTitle();
    hmeasptrack_vs_hits->SetDirectory(m_dir_main);

    hmeasptrack_vs_chi2perNDF = new TH2D("hmeasptrack_vs_chi2perNDF","",50,0,50,10,0,10);
    hmeasptrack_vs_chi2perNDF->GetXaxis()->SetTitle("Track #Chi^{2} Sum/NDF");hmeasptrack_vs_chi2perNDF->GetXaxis()->CenterTitle();
    hmeasptrack_vs_chi2perNDF->GetYaxis()->SetTitle("Number of Measurements per Track");hmeasptrack_vs_chi2perNDF->GetYaxis()->CenterTitle();
    hmeasptrack_vs_chi2perNDF->SetDirectory(m_dir_main);

    hmeasptrack_vs_calstates = new TH2D("hmeasptrack_vs_calstates","",15,0,15,15,0,15);
    hmeasptrack_vs_calstates->GetXaxis()->SetTitle("Number of Measurements per Track");hmeasptrack_vs_calstates->GetXaxis()->CenterTitle();
    hmeasptrack_vs_calstates->GetYaxis()->SetTitle("Number of Calibrated States");hmeasptrack_vs_calstates->GetYaxis()->CenterTitle();
    hmeasptrack_vs_calstates->SetDirectory(m_dir_main);

    hmeaschi2_vs_chi2 = new TH2D("hmeaschi2_vs_chi2","",50,0,50,50,0,20);
    hmeaschi2_vs_chi2->GetXaxis()->SetTitle("Track #Chi^{2} Sum");hmeaschi2_vs_chi2->GetXaxis()->CenterTitle();
    hmeaschi2_vs_chi2->GetYaxis()->SetTitle("#Chi^{2} Individual Measurements");hmeaschi2_vs_chi2->GetYaxis()->CenterTitle();
    hmeaschi2_vs_chi2->SetDirectory(m_dir_main);

    hmeaschi2_vs_eta = new TH2D("hmeaschi2_vs_eta","",50,-4,4,50,0,20);
    hmeaschi2_vs_eta->GetXaxis()->SetTitle("#eta");hmeaschi2_vs_eta->GetXaxis()->CenterTitle();
    hmeaschi2_vs_eta->GetYaxis()->SetTitle("#Chi^{2} Individual Measurements");hmeaschi2_vs_eta->GetYaxis()->CenterTitle();
    hmeaschi2_vs_eta->SetDirectory(m_dir_main);

    hmeaschi2_vs_hits = new TH2D("hmeaschi2_vs_hits","",50,0,50,50,0,20);
    hmeaschi2_vs_hits->GetXaxis()->SetTitle("Number of Hits");hmeaschi2_vs_hits->GetXaxis()->CenterTitle();
    hmeaschi2_vs_hits->GetYaxis()->SetTitle("#Chi^{2} Individual Measurements");hmeaschi2_vs_hits->GetYaxis()->CenterTitle();
    hmeaschi2_vs_hits->SetDirectory(m_dir_main);

    const int n_eta_bins = 16;
    TVectorT<double> V_eta_edges(n_eta_bins+1);
    printf("%d", V_eta_edges[0]);
    //use 0.5i-4 to get lowerbound and 0.5i-3.5 to get upper bound
    for (int i=0; i<n_eta_bins; i++){
        double low_eta = 0.5*i-4;
        double high_eta = 0.5*i-3.5;
        V_eta_edges[i] = low_eta;
        V_eta_edges[i+1] = high_eta;
    
	    TH2 *htemp = new TH2D(TString::Format("hchi2_vs_hits_eta_%.1f_%.1f", low_eta, high_eta),
                    "",50,0,50,50,0,50);
        htemp->GetXaxis()->SetTitle("Number of Hits");htemp->GetXaxis()->CenterTitle();
        htemp->GetYaxis()->SetTitle("Track #Chi^{2} Sum");htemp->GetYaxis()->CenterTitle();
        hchi2_vs_hits_etabins.push_back(htemp);
        hchi2_vs_hits_etabins[i]->SetDirectory(m_dir_sub);

        TH2 *htemp1 = new TH2D(TString::Format("hmeasptrack_vs_hits_eta_%.1f_%.1f", low_eta, high_eta),
                    "",50,0,50,10,0,10);
        htemp1->GetXaxis()->SetTitle("Number of Hits");htemp1->GetXaxis()->CenterTitle();
        htemp1->GetYaxis()->SetTitle("Number of Measurements per Track");htemp1->GetYaxis()->CenterTitle();
        hmeasptrack_vs_hits_etabins.push_back(htemp1);
        hmeasptrack_vs_hits_etabins[i]->SetDirectory(m_dir_sub);

        TH2 *htemp1b = new TH2D(TString::Format("hmeasptrack_vs_hits_eta_zoomed_%.1f_%.1f", low_eta, high_eta), 
                    "",10,0,10,10,0,10);
        htemp1b->GetXaxis()->SetTitle("Number of Hits");htemp1b->GetXaxis()->CenterTitle();
        htemp1b->GetYaxis()->SetTitle("Number of Measurements per Track");htemp1b->GetYaxis()->CenterTitle();
        hmeasptrack_vs_hits_etabins_zoomed.push_back(htemp1b);
        hmeasptrack_vs_hits_etabins_zoomed[i]->SetDirectory(m_dir_sub);

        TH2 *htemp2 = new TH2D(TString::Format("hmeasptrack_vs_chi2perNDF_eta_%.1f_%.1f", low_eta, high_eta), 
                    "",50,0,50,10,0,10);
        htemp2->GetXaxis()->SetTitle("Track #Chi^{2} Sum/NDF");htemp2->GetXaxis()->CenterTitle();
        htemp2->GetYaxis()->SetTitle("Number of Measurements per Track");htemp2->GetYaxis()->CenterTitle();
        hmeasptrack_vs_chi2perNDF_etabins.push_back(htemp2);
        hmeasptrack_vs_chi2perNDF_etabins[i]->SetDirectory(m_dir_sub);
    }
    // hchi2_vs_hits_etabins->SetDirectory(m_dir_main);


    hmeaschi2_vs_volID = new TH2D("hmeaschi2_vs_volID","",50,0,50,50,0,20);
    hmeaschi2_vs_volID->GetXaxis()->SetTitle("Volume ID");hmeaschi2_vs_volID->GetXaxis()->CenterTitle();
    hmeaschi2_vs_volID->GetYaxis()->SetTitle("#Chi^{2} Individual Measurements");hmeaschi2_vs_volID->GetYaxis()->CenterTitle();
    hmeaschi2_vs_volID->SetDirectory(m_dir_main);

    hmeaschi2_vs_layID = new TH2D("hmeaschi2_vs_layID","",50,0,50,50,0,20);
    hmeaschi2_vs_layID->GetXaxis()->SetTitle("Layer ID");hmeaschi2_vs_layID->GetXaxis()->CenterTitle();
    hmeaschi2_vs_layID->GetYaxis()->SetTitle("#Chi^{2} Individual Measurements");hmeaschi2_vs_layID->GetYaxis()->CenterTitle();
    hmeaschi2_vs_layID->SetDirectory(m_dir_main);

    hmeaschi2_vs_vollayIDs = new TH2D("hmeaschi2_vs_vollayIDs","",400,0,400,50,0,20);
    hmeaschi2_vs_vollayIDs->GetXaxis()->SetTitle("Volume ID * 10 + Layer ID");hmeaschi2_vs_vollayIDs->GetXaxis()->CenterTitle();
    hmeaschi2_vs_vollayIDs->GetYaxis()->SetTitle("#Chi^{2} Individual Measurements");hmeaschi2_vs_vollayIDs->GetYaxis()->CenterTitle();
    hmeaschi2_vs_vollayIDs->SetDirectory(m_dir_main);
    

    file -> cd();
    V_eta_edges.Write("V_eta_edges");

    hholes_vs_hits = new TH2D("hholes_vs_hits","",50,0,50,5,0,5);
    hholes_vs_hits->GetXaxis()->SetTitle("Number of Hits");hholes_vs_hits->GetXaxis()->CenterTitle();
    hholes_vs_hits->GetYaxis()->SetTitle("Number of Holes");hholes_vs_hits->GetYaxis()->CenterTitle();
    hholes_vs_hits->SetDirectory(m_dir_main);

    houtliers_vs_hits = new TH2D("houtliers_vs_hits","",50,0,50,5,0,5);
    houtliers_vs_hits->GetXaxis()->SetTitle("Number of Hits");houtliers_vs_hits->GetXaxis()->CenterTitle();
    houtliers_vs_hits->GetYaxis()->SetTitle("Number of Outliers");houtliers_vs_hits->GetYaxis()->CenterTitle();
    houtliers_vs_hits->SetDirectory(m_dir_main);

    hsummation = new TH2D("hsummation","",15,0,15,15,0,15);
    hsummation->GetXaxis()->SetTitle("Number of Meas per Track + Number of Outliers");hsummation->GetXaxis()->CenterTitle();
    hsummation->GetYaxis()->SetTitle("Number of Calibrated States");hsummation->GetYaxis()->CenterTitle();
    hsummation->SetDirectory(m_dir_main);

    hsummation2 = new TH2D("hsummation2","",50,0,50,50,0,50);
    hsummation2->GetXaxis()->SetTitle("Number of Meas per Track + Number of Outliers");hsummation2->GetXaxis()->CenterTitle();
    hsummation2->GetYaxis()->SetTitle("Number of Hits");hsummation2->GetYaxis()->CenterTitle();
    hsummation2->SetDirectory(m_dir_main);

    hsummation3 = new TH2D("hsummation3","",15,0,15,15,0,15);
    hsummation3->GetXaxis()->SetTitle("Number of Meas per Track + Number of Outliers + Number of Holes");hsummation3->GetXaxis()->CenterTitle();
    hsummation3->GetYaxis()->SetTitle("Number of Calibrated States");hsummation3->GetYaxis()->CenterTitle();
    hsummation3->SetDirectory(m_dir_main);

    // TString eta_regions[] = {"backward", "barrel", "forward"};
    char eta_regions[3][20] = {"backward", "barrel", "forward"};
    for (int i=0; i<3; i++){
    
	    TH1 *htemp = new TH1D(TString::Format("hhits_in_r_%s", eta_regions[i]),
                    "",2000,-1000,1000);
        htemp->GetXaxis()->SetTitle("r [mm]");htemp->GetXaxis()->CenterTitle();
        htemp->GetYaxis()->SetTitle("Counts");htemp->GetYaxis()->CenterTitle();
        htemp->SetLineWidth(2);htemp->SetLineColor(kBlue);
        hhits_in_r.push_back(htemp);
        hhits_in_r[i]->SetDirectory(m_dir_res);

        TH1 *htemp1 = new TH1D(TString::Format("hhits_in_z_%s", eta_regions[i]),
                    "",4000,-2000,2000);
        htemp1->GetXaxis()->SetTitle("z [mm]");htemp1->GetXaxis()->CenterTitle();
        htemp1->GetYaxis()->SetTitle("Counts");htemp1->GetYaxis()->CenterTitle();
        htemp1->SetLineWidth(2);htemp1->SetLineColor(kBlue);
        hhits_in_z.push_back(htemp1);
        hhits_in_z[i]->SetDirectory(m_dir_res);

        TH2 *htemp2 = new TH2D(TString::Format("hhits_r_vs_z_%s", eta_regions[i]),"",4000,-2000,2000,2000,-1000,1000);
        htemp2->GetXaxis()->SetTitle("z [mm]");htemp2->GetXaxis()->CenterTitle();
        htemp2->GetYaxis()->SetTitle("r [mm]");htemp2->GetYaxis()->CenterTitle();
        hhits_r_vs_z.push_back(htemp2);
        hhits_r_vs_z[i]->SetDirectory(m_dir_res);

        
        TH1 *htemp3 = new TH1D(TString::Format("hmeas_in_r_%s", eta_regions[i]),
                    "",2000,-1000,1000);
        htemp3->GetXaxis()->SetTitle("r (measurements) [mm]");htemp3->GetXaxis()->CenterTitle();
        htemp3->GetYaxis()->SetTitle("Counts");htemp3->GetYaxis()->CenterTitle();
        htemp3->SetLineWidth(2);htemp3->SetLineColor(kRed); htemp3->SetLineStyle(7);
        hmeas_in_r.push_back(htemp3);
        hmeas_in_r[i]->SetDirectory(m_dir_res);

        TH1 *htemp4 = new TH1D(TString::Format("hmeas_in_z_%s", eta_regions[i]),
                    "",4000,-2000,2000);
        htemp4->GetXaxis()->SetTitle("z (measurements) [mm]");htemp4->GetXaxis()->CenterTitle();
        htemp4->GetYaxis()->SetTitle("Counts");htemp4->GetYaxis()->CenterTitle();
        htemp4->SetLineWidth(2);htemp4->SetLineColor(kRed); htemp4->SetLineStyle(7);
        hmeas_in_z.push_back(htemp4);
        hmeas_in_z[i]->SetDirectory(m_dir_res);
        
        TH2 *htemp5 = new TH2D(TString::Format("hmeas_r_vs_z_%s", eta_regions[i]),"",4000,-2000,2000,2000,-1000,1000);
        htemp5->GetXaxis()->SetTitle("z [mm]");htemp5->GetXaxis()->CenterTitle();
        htemp5->GetYaxis()->SetTitle("r [mm]");htemp5->GetYaxis()->CenterTitle();
        hmeas_r_vs_z.push_back(htemp5);
        hmeas_r_vs_z[i]->SetDirectory(m_dir_res);

        TH1 *htemp6 = new TH1D(TString::Format("hresiduals_%s", eta_regions[i]),"Residuals",400,-0.5,0.5);
        if (i==1) htemp6->GetXaxis()->SetTitle("#Delta z_{(track meas - hit)} [mm]"); else {htemp6->GetXaxis()->SetTitle("#Delta r_{(track meas - hit)} [mm]");}
        htemp6->GetXaxis()->CenterTitle();
        htemp6->GetYaxis()->SetTitle("Counts");htemp6->GetYaxis()->CenterTitle();
        htemp6->SetLineWidth(2);htemp6->SetLineColor(kBlue);
        hresiduals.push_back(htemp6);
        hresiduals[i]->SetDirectory(m_dir_res);

        TH2 *htemp7 = new TH2D(TString::Format("hmeas_outliers_r_vs_z_%s", eta_regions[i]), 
                    "Trackstate Outliers",4000,-2000,2000,2000,-1000,1000);
        htemp7->GetXaxis()->SetTitle("z (track state meas) [mm]");htemp7->GetXaxis()->CenterTitle();
        htemp7->GetYaxis()->SetTitle("r (track state meas) [mm]");htemp7->GetYaxis()->CenterTitle();
        hmeas_outliers_r_vs_z.push_back(htemp7);
        hmeas_outliers_r_vs_z[i]->SetDirectory(m_dir_res);

        TH2 *htemp8 = new TH2D(TString::Format("hmeas_holes_r_vs_z_%s", eta_regions[i]), 
                    "Trackstate Holes",4000,-2000,2000,2000,-1000,1000);
        htemp8->GetXaxis()->SetTitle("z (track state meas) [mm]");htemp8->GetXaxis()->CenterTitle();
        htemp8->GetYaxis()->SetTitle("r (track state meas) [mm]");htemp8->GetYaxis()->CenterTitle();
        hmeas_holes_r_vs_z.push_back(htemp8);
        hmeas_holes_r_vs_z[i]->SetDirectory(m_dir_res);
    }

    //Looking at volume + layer IDs
    hvolID = new TH1D("hvolID","",50,0,50);
    hvolID->GetXaxis()->SetTitle("Volume ID");hvolID->GetXaxis()->CenterTitle();
    hvolID->GetYaxis()->SetTitle("Counts");hvolID->GetYaxis()->CenterTitle();
    hvolID->SetLineWidth(2);hvolID->SetLineColor(kBlue);
    hvolID->SetDirectory(m_dir_main);

    hlayID = new TH1D("hlayID","",50,0,50);
    hlayID->GetXaxis()->SetTitle("Layer ID");hlayID->GetXaxis()->CenterTitle();
    hlayID->GetYaxis()->SetTitle("Counts");hlayID->GetYaxis()->CenterTitle();
    hlayID->SetLineWidth(2);hlayID->SetLineColor(kBlue);
    hlayID->SetDirectory(m_dir_main);

    hvollayIDs = new TH1D("hvollayIDs","",400,0,400);
    hvollayIDs->GetXaxis()->SetTitle("Volume ID * 10 + Layer ID");hvollayIDs->GetXaxis()->CenterTitle();
    hvollayIDs->GetYaxis()->SetTitle("Counts");hvollayIDs->GetYaxis()->CenterTitle();
    hvollayIDs->SetLineWidth(2);hvollayIDs->SetLineColor(kBlue);
    hvollayIDs->SetDirectory(m_dir_main);
    int NUM_LAYERS = 20;
    for (int i=0; i<NUM_LAYERS; i++){

        //vollay_index[vollay[i]] = i; //assign index to each item of vollay array
        vollay_index.emplace(vollay_arr[i], i); //assign index to each item of vollay array
    
	    // TH2 *htemp = new TH2D(TString::Format("htrackstate_r_vs_vollayIDs_%d", vollay_arr[i]), 
        //             "",400,0,400,2000,-1000,1000);
        // htemp->GetXaxis()->SetTitle("Volume ID * 10 + Layer ID");htemp->GetXaxis()->CenterTitle();
        // htemp->GetYaxis()->SetTitle("r (track state meas) [mm]");htemp->GetYaxis()->CenterTitle();
        // htrackstate_r_vs_vollayIDs.push_back(htemp);
        // htrackstate_r_vs_vollayIDs[i]->SetDirectory(m_dir_res);

        // TH2 *htemp1 = new TH2D(TString::Format("htrackstate_z_vs_vollayIDs_%d", vollay_arr[i]), 
        //             "",400,0,400,4000,-2000,2000);
        // htemp1->GetXaxis()->SetTitle("Volume ID * 10 + Layer ID");htemp1->GetXaxis()->CenterTitle();
        // htemp1->GetYaxis()->SetTitle("z (track state meas) [mm]");htemp1->GetYaxis()->CenterTitle();
        // htrackstate_z_vs_vollayIDs.push_back(htemp1);
        // htrackstate_z_vs_vollayIDs[i]->SetDirectory(m_dir_res);

        TH1 *htemp2 = new TH1D(TString::Format("htrackstate_r_%d", vollay_arr[i]),
                    "Trackstate msmts",2000,-1000,1000);
        htemp2->GetXaxis()->SetTitle("r (measurements) [mm]");htemp2->GetXaxis()->CenterTitle();
        htemp2->GetYaxis()->SetTitle("Counts");htemp2->GetYaxis()->CenterTitle();
        htemp2->SetLineWidth(2);htemp2->SetLineColor(kBlue); htemp2->SetLineStyle(7);
        htrackstate_r.push_back(htemp2);
        htrackstate_r[i]->SetDirectory(m_dir_res);

        TH1 *htemp3 = new TH1D(TString::Format("htrackstate_z_%d", vollay_arr[i]),
                    "Trackstate msmts",4000,-2000,2000);
        htemp3->GetXaxis()->SetTitle("z (measurements) [mm]");htemp3->GetXaxis()->CenterTitle();
        htemp3->GetYaxis()->SetTitle("Counts");htemp3->GetYaxis()->CenterTitle();
        htemp3->SetLineWidth(2);htemp3->SetLineColor(kBlue); htemp3->SetLineStyle(7);
        htrackstate_z.push_back(htemp3);
        htrackstate_z[i]->SetDirectory(m_dir_res);

        
        TH2 *htemp4 = new TH2D(TString::Format("htrackstate_r_vs_z_%d", vollay_arr[i]), 
                    "Trackstate msmts",4000,-2000,2000,2000,-1000,1000);
        htemp4->GetXaxis()->SetTitle("z (track state meas) [mm]");htemp4->GetXaxis()->CenterTitle();
        htemp4->GetYaxis()->SetTitle("r (track state meas) [mm]");htemp4->GetYaxis()->CenterTitle();
        htrackstate_r_vs_z.push_back(htemp4);
        htrackstate_r_vs_z[i]->SetDirectory(m_dir_res);

        TH1 *htemp5 = new TH1D(TString::Format("hresiduals_vollaybins_%d", vollay_arr[i]),
                    "Residuals",200,-0.5,0.5);
        if (i == 7 || i == 8 ||  i == 9 ||  i == 11 ||  i == 13 ||  i == 17 || i == 18 ) htemp5->GetXaxis()->SetTitle("#Delta z_{(track meas - hit)} [mm]"); 
        else {htemp5->GetXaxis()->SetTitle("#Delta r_{(track meas - hit)} [mm]");}
        htemp5->GetXaxis()->CenterTitle();
        htemp5->GetYaxis()->SetTitle("Counts");htemp5->GetYaxis()->CenterTitle();
        htemp5->SetLineWidth(2);htemp5->SetLineColor(kBlue); //htemp5->SetLineStyle(7);
        hresiduals_vollaybins.push_back(htemp5);
        hresiduals_vollaybins[i]->SetDirectory(m_dir_res);


    }

    for (int i=0; i<NUM_LAYERS; i++) {
        hresiduals_layers_in_pbins.push_back(vector<TH1*>());
        for (int j=0; j<4; j++) { //momentum bins
            TH1 *htemp = new TH1D(TString::Format("hresiduals_vollay_%d_mom_%s", vollay_arr[i], mom_bins_arr[j]),
                        "Residuals",200,-0.5,0.5);
            if (i == 7 || i == 8 ||  i == 9 ||  i == 11 ||  i == 13 ||  i == 17 || i == 18 ) htemp->GetXaxis()->SetTitle("#Delta z_{(track meas - hit)} [mm]"); 
            else {htemp->GetXaxis()->SetTitle("#Delta r_{(track meas - hit)} [mm]");}
            htemp->GetXaxis()->CenterTitle();
            htemp->GetYaxis()->SetTitle("Counts");htemp->GetYaxis()->CenterTitle();
            htemp->SetLineWidth(2);htemp->SetLineColor(kBlue); //htemp->SetLineStyle(7);
            hresiduals_layers_in_pbins[i].push_back(htemp);
            hresiduals_layers_in_pbins[i][j]->SetDirectory(m_dir_res);
            
        }    
    }

    // Get log level from user parameter or default
    InitLogger(plugin_name);

    auto acts_service = app->GetService<ACTSGeo_service>();
    m_geo_provider = acts_service->actsGeoProvider();

    test_counter = 0;

}


//------------------
// Process
//------------------
// This function is called every event
void trackqa_processor::Process(const std::shared_ptr<const JEvent>& event)
{

    m_log->trace("");
    m_log->trace("trackqa_processor event");

    //Generated particles (only one particle generated)
    double mceta = 0; //particle eta
    double mcphi = 0; //particle phi
    double mcp = 0; //total momentum
    double mcpt = 0; //transverse momentum
    double mce = 0; //energy
    int mcid = 0; //PDG id
    int num_primary = 0; //Number of primary particles

    auto mcParticles = event->Get<edm4hep::MCParticle>("MCParticles");

    for( size_t iParticle=0;iParticle<mcParticles.size(); iParticle++ ){
	
	    auto mcparticle = mcParticles[iParticle];
        if(mcparticle->getGeneratorStatus() != 1) continue;
        auto& mom = mcparticle->getMomentum();
        
        mceta = -log(tan(atan2(sqrt(mom.x*mom.x+mom.y*mom.y),mom.z)/2.));
        mcphi = atan2(mom.y, mom.x);
	    mcp = sqrt(mom.x*mom.x+mom.y*mom.y+mom.z*mom.z);
        mcpt = sqrt(mom.x*mom.x+mom.y*mom.y);
        mce = sqrt(mcp*mcp + mcparticle->getMass()*mcparticle->getMass());	

        mcid = mcparticle->getPDG();

	    num_primary++;

    }

    //Print generated info to log
    m_log->trace("-------------------------");
    m_log->trace("Number of primary generated particles:");
    m_log->trace("{:>10}",num_primary);
    m_log->trace("Generated particle id, eta, p, E:");
    m_log->trace("{:>10} {:>10.2f} {:>10.2f} {:>10.2f}", mcid, mceta, mcp, mce);

    //Tracker hits
    std::vector<std::string> m_data_names = {
        "SiBarrelTrackerRecHits",         // Barrel Tracker
        "SiBarrelVertexRecHits",          // Vertex
        "SiEndcapTrackerRecHits",         // End Cap tracker
        "MPGDBarrelRecHits",              // MPGD
        "MPGDDIRCRecHits",                // MPGD DIRC
        "TOFBarrelRecHit",                // Barrel TOF
        "TOFEndcapRecHits"                // End Cap TOF
    };

    m_log->trace("-------------------------");
    int nHitsallTrackers = 0;
    // int index_reg = 0; if (mceta >= -0.5 && mceta < 0.5) index_reg = 1; else if (mceta >= 0.5) index_reg = 2;
    int index_reg = 0; if (mceta >= -0.88137 && mceta < 0.88137) index_reg = 1; else if (mceta >= 0.88137) index_reg = 2;
    vector<float> r_hits_arr;
    vector<float> z_hits_arr; 
    
    for(size_t name_index = 0; name_index < m_data_names.size(); name_index++ ) {
        auto data_name = m_data_names[name_index];
        auto hits = event->Get<edm4eic::TrackerHit>(data_name);
        m_log->trace("Detector {} has {} digitized hits.",data_name,hits.size());
        
        int nHits_detector = 0;
        // int index_reg = 0; if (mceta >= -1. && mceta < 1.) index_reg = 1; else if (mceta >= 1.) index_reg = 2;
        for(auto hit: hits) {
            auto cell_id = hit->getCellID(); //FIXME: convert to volume id and layer id
            auto x = hit->getPosition().x;
            auto y = hit->getPosition().y;
            auto z = hit->getPosition().z;
            auto r = sqrt(x*x + y*y);
            auto etahit = -log(tan(atan2(r,z)/2.));

            nHits_detector++;
            m_log->trace("For digitized hit number {}:",nHits_detector);
            m_log->trace("Cell Id is {}",cell_id);
            m_log->trace("Hit x, y, z, r, eta:");
            m_log->trace("{:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f}",x,y,z,r,etahit);
            nHitsallTrackers++;

            //Fill hists:
            if(num_primary==1){
                hhits_in_r[index_reg]->Fill(r);
                hhits_in_z[index_reg]->Fill(z);
                hhits_r_vs_z[index_reg]->Fill(z,r);

                r_hits_arr.push_back(r);
                z_hits_arr.push_back(z);
            }
        }

        m_log->trace("");
    }

    m_log->trace("Total number of tracker hits is {}",nHitsallTrackers);
    m_log->trace("-------------------------");

    //Print out number of seeds from orthogonal seeder
    auto seed_parameters = event->Get<eicrecon::TrackParameters>("SeededTrackParams");
    m_log->trace("Number of ACTS Seeds: {}", seed_parameters.size());

    //ACTS Trajectories
    auto trajectories = event->Get<eicrecon::TrackingResultTrajectory>("CentralCKFTrajectories"); //for truth-seeded tracjectories
    //auto trajectories = event->Get<eicrecon::TrackingResultTrajectory>("CentralCKFSeededTrajectories"); // for realistic-seeded trajectories

    m_log->trace("Number of ACTS Trajectories: {}", trajectories.size());
    m_log->trace("");

    // Loop over the trajectories
    int num_traj = 0;
    
    // //Populate r_measurements_arr
    // //Populate z_measurements_arr
    vector<float> r_measurements_arr;
    vector<float> z_measurements_arr;
    vector<int> vollayids;
    vector<int> pbins;
    int indexregsaved;
    for (const auto& traj : trajectories) {
        // Get the entry index for the single trajectory
        // The trajectory entry indices and the multiTrajectory
        const auto &mj = traj->multiTrajectory();
        const auto &trackTips = traj->tips();

        // Skip empty
        if (trackTips.empty()) {
            m_log->trace("Empty multiTrajectory.");
            continue;
        }
        auto &trackTip = trackTips.front();

        // Collect the trajectory summary info  
        auto trajState = Acts::MultiTrajectoryHelpers::trajectoryState(mj, trackTip);
        int m_nStates = trajState.nStates;
        int m_nMeasurements = trajState.nMeasurements;
        int m_nOutliers = trajState.nOutliers;
        auto m_chi2Sum = trajState.chi2Sum;
        int m_NDF = trajState.NDF;
        auto m_measurementChi2 = trajState.measurementChi2;
        int m_nHoles = trajState.nHoles;

        //General Trajectory Information
        m_log->trace("Number of elements in trackTips {}", trackTips.size());
        m_log->trace("Number of states in trajectory     : {}", m_nStates);
        m_log->trace("Number of measurements in trajectory: {}", m_nMeasurements);
        m_log->trace("Number of outliers in trajectory     : {}", m_nOutliers);
        m_log->trace("Total chi-square of trajectory    : {:>8.2f}",m_chi2Sum);
   
        //Initial Trajectory Parameters
        const auto &initial_bound_parameters = traj->trackParameters(trackTip);
        const auto &parameter  = initial_bound_parameters.parameters();
        const auto &covariance = *initial_bound_parameters.covariance();

        m_log->trace("{:>8} {:>8} {:>8} {:>8} {:>8} {:>10} {:>10} {:>10}",
                    "[loc 0]","[loc 1]", "[phi]", "[theta]", "[q/p]", "[err phi]", "[err th]", "[err q/p]");
        m_log->trace("{:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f} {:>10.2g} {:>10.2g} {:>10.2g}",
                    parameter[Acts::eBoundLoc0],
                    parameter[Acts::eBoundLoc1],
                    parameter[Acts::eBoundPhi],
                    parameter[Acts::eBoundTheta],
                    parameter[Acts::eBoundQOverP],
                    sqrt(covariance(Acts::eBoundPhi, Acts::eBoundPhi)),
                    sqrt(covariance(Acts::eBoundTheta, Acts::eBoundTheta)),
                    sqrt(covariance(Acts::eBoundQOverP, Acts::eBoundQOverP))
                    );

        auto p_traj = fabs(1. / parameter[Acts::eBoundQOverP]);
        auto eta_traj = -log( tan(parameter[Acts::eBoundTheta]/2.) );
        m_log->trace("Trajectory p, eta:");
        m_log->trace("{:>10.2f} {:>10.2f}",p_traj,eta_traj);
        m_log->trace("");
        // int index_reg = 0; if (eta_traj >= -0.88137 && eta_traj < 0.88137) index_reg = 1; else if (eta_traj >= 0.88137) index_reg = 2;
        // int index_reg = 0; if (eta_traj >= -0.5 && eta_traj < 0.5) index_reg = 1; else if (eta_traj >= 0.5) index_reg = 2;


        //Information at tracking layers
        int m_nCalibrated = 0;
        int state_counter = 0;
        int numCalState = -1;
        // visit the track points
        mj.visitBackwards(trackTip, [&](auto &&trackstate) {
            
            auto typeFlags = trackstate.typeFlags();
            // if (typeFlags.test(Acts::TrackStateFlag::OutlierFlag) == 1) 
            // cout << "FLAG!!: " << typeFlags.test(Acts::TrackStateFlag::OutlierFlag) << endl;

            state_counter++;
            m_log->trace("Now at State number {}",state_counter);

            // get volume info
            auto geoID = trackstate.referenceSurface().geometryId();
            auto volume = geoID.volume();
            auto layer = geoID.layer();
            m_log->trace("Volume id is {}, layer id is {}",volume,layer);

            if (trackstate.hasCalibrated()) {
                m_nCalibrated++;
                numCalState++;
                m_log->trace("This is a calibrated state.");
            }
            else{
                m_log->trace("This is NOT a calibrated state.");
            }

            // get track state parameters and their covariances
            const auto &state_params = trackstate.predicted();
            const auto &state_covar = trackstate.predictedCovariance();

            // //get local  hit residuals
            // cout << "begin" << endl;
            // cout << "H: " << trackstate.effectiveProjector() <<endl;
            // cout << "eff cal: " << trackstate.effectiveCalibrated() <<endl;
            // cout << "eff cal cov: " << trackstate.effectiveCalibratedCovariance() <<endl;
            // cout << "ststae params: " << state_params <<endl;
            // cout << "cov: " << state_covar <<endl;
            // cout << "H transposed: " << trackstate.effectiveProjector().transpose() <<endl;

            // auto typeFlags = trackstate.typeFlags();

            // auto H = trackstate.effectiveProjector();
            // auto resCov = trackstate.effectiveCalibratedCovariance() + H * state_covar * H.transpose();
            // auto res = trackstate.effectiveCalibrated() - (H * state_params);
            // cout << "res calculated" << endl;
            // cout << Acts::eBoundLoc0 << endl;
            // cout << " siZe pf: " << sizeof(res) << endl;
            // cout << " type: " << typeid(res).name() << endl;
            // // for (int ij=0; ij<sizeof(res); ij++){
            // //     cout << res[ij] <<", " << endl;
            // // }
            // cout << "testing" << state_params[0] <<endl;
            // cout << "res: " << res << endl;
            // cout << "calibrated size: " << trackstate.calibratedSize() << endl;
            // cout << "res.rows: " << res.rows() << endl;
            // cout << "res.cols: " << res.cols() << endl;
            // // cout << "res(0,0): " << res(0,0) << endl;
            // cout << "res.coeff(0,0): " << res.coeff(0,0) << endl;
            // if (typeFlags.test(Acts::TrackStateFlag::MeasurementFlag)) {
            //     cout << "res[0]: " << res[1] << endl;
            //     cout << "value:" << res[Acts::eBoundLoc0] << endl;
            //     // m_res_x_hit.push_back(res[Acts::eBoundLoc0]);
            // }
            


            //First print same information as for initial track parameters
            m_log->trace("{:>8} {:>8} {:>8} {:>8} {:>8} {:>10} {:>10} {:>10}",
                    "[loc 0]","[loc 1]", "[phi]", "[theta]", "[q/p]", "[err phi]", "[err th]", "[err q/p]");
            m_log->trace("{:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f} {:>10.2g} {:>10.2g} {:>10.2g}",
                    state_params[Acts::eBoundLoc0],
                    state_params[Acts::eBoundLoc1],
                    state_params[Acts::eBoundPhi],
                    state_params[Acts::eBoundTheta],
                    state_params[Acts::eBoundQOverP],
                    sqrt(state_covar(Acts::eBoundPhi, Acts::eBoundPhi)),
                    sqrt(state_covar(Acts::eBoundTheta, Acts::eBoundTheta)),
                    sqrt(state_covar(Acts::eBoundQOverP, Acts::eBoundQOverP))
                    );

            // convert local to global
            auto global = trackstate.referenceSurface().localToGlobal(
                    m_geo_provider->getActsGeometryContext(),
                    {state_params[Acts::eBoundLoc0], state_params[Acts::eBoundLoc1]},
                    {0, 0, 0} );
            auto global_r = sqrt(global.x()*global.x()+global.y()*global.y());
            m_log->trace("State global x, y, z, r and pathlength:");
            m_log->trace("{:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f}",global.x(),global.y(),global.z(),global_r,trackstate.pathLength());
            
            m_log->trace("");
            //Fill histograms - note there are only meas chi^2 associated with the CALIBRATED states
            if(num_primary==1 && trackstate.hasCalibrated()){
                hmeaschi2_vs_volID->Fill(volume,m_measurementChi2[numCalState]);
                hmeaschi2_vs_layID->Fill(layer,m_measurementChi2[numCalState]);
                hmeaschi2_vs_vollayIDs->Fill(volume*10+layer,m_measurementChi2[numCalState]);

                hmeas_in_r[index_reg]->Fill(global_r);
                hmeas_in_z[index_reg]->Fill(global.z());
                hmeas_r_vs_z[index_reg]->Fill(global.z(), global_r);
                if (typeFlags.test(Acts::TrackStateFlag::OutlierFlag) == 1) hmeas_outliers_r_vs_z[index_reg]->Fill(global.z(), global_r);
                if (typeFlags.test(Acts::TrackStateFlag::HoleFlag) == 1) hmeas_holes_r_vs_z[index_reg]->Fill(global.z(), global_r);

                hvolID->Fill(volume);
                hlayID->Fill(layer);
                hvollayIDs->Fill(volume*10+layer);

                printf("vollayget = %d\n", volume * 10 + layer);
                int vollayID = vollay_index[volume * 10 + layer];
                auto itr = find(vollay_arr, vollay_arr+20, volume*10+layer);
                //int vollayID = distance(vollay_arr, itr); //DO NOT USE
                // htrackstate_r_vs_vollayIDs[0]->Fill(volume*10+layer,global_r); //index 0 holds all events (summary)
                // htrackstate_r_vs_vollayIDs[vollayID]->Fill(volume*10+layer,global_r);
                // htrackstate_z_vs_vollayIDs[0]->Fill(volume*10+layer,global.z()); //index 0 holds all events (summary)
                // htrackstate_z_vs_vollayIDs[vollayID]->Fill(volume*10+layer,global.z()); 

                htrackstate_r[0]->Fill(global_r);

                htrackstate_r[vollayID]->Fill(global_r);

                htrackstate_z[0]->Fill(global.z());
                htrackstate_z[vollayID]->Fill(global.z());

                htrackstate_r_vs_z[0]->Fill(global.z(),global_r);
                htrackstate_r_vs_z[vollayID]->Fill(global.z(),global_r);
                
                //get measurement vectors
                r_measurements_arr.push_back(global_r);
                z_measurements_arr.push_back(global.z());
                vollayids.push_back(vollayID);
                int pbin = 3;
                if (mcp > 1. && mcp < 2.) pbin = 0; else if (mcp < 5.) pbin = 1; else if (mcp < 7.) pbin = 2;
                printf("Pbin is currently: %d\n", pbin);
                printf("VollayID is currently: %d\n", vollayID);
                pbins.push_back(pbin);
                indexregsaved = index_reg;
                
                // // calculate residuals
                // if (index_reg == 1) { //this is in the barrel - expect r's to be the same, look at dif in z
                //     //look at the hits in this event, see which ones have the same r's
                //     for (int jj=0; jj<r_hits_arr.size(); jj++){
                //         if (fabs(global_r - r_hits_arr[jj]) < 1. ){ //mm
                //             hresiduals[index_reg]->Fill(global.z() - z_hits_arr[jj]);
                //             // break;
                //         }
                //     }
                // } else { //this is in the forward/backward - expect z's to be the same, look at dif in r
                //     //look at the hits in this event, see which ones have the same z's
                //     for (int jj=0; jj<z_hits_arr.size(); jj++){
                //         if (fabs(global.z() - z_hits_arr[jj]) < 1. ){
                //             hresiduals[index_reg]->Fill( global_r - r_hits_arr[jj]);
                //             // break;
                //         }
                //     }
                // }

                // int pbin = 3;
                // if (mcp > 1. && mcp < 2.) pbin = 0; else if (mcp < 5.) pbin = 1; else if (mcp < 7.) pbin = 2;
                // if (vollayID == 7 || vollayID == 8 ||  vollayID == 9 ||  vollayID == 11 ||  vollayID == 13 ||  vollayID == 17 || vollayID == 18 ) {
                //     //constant r if vertex/sagitta/barrel -> indices are 7,8,9,11,13,17,18
                //     for (int jj=0; jj<r_hits_arr.size(); jj++){
                //         if (fabs(global_r - r_hits_arr[jj]) < 1. ){ //mm
                //             hresiduals_vollaybins[vollayID]->Fill( global.z() - z_hits_arr[jj]);
                //             hresiduals_layers_in_pbins[vollayID][pbin]->Fill( global.z() - z_hits_arr[jj]);
                //             // break;
                //         }
                //     }
                // } else {
                //     for (int jj=0; jj<z_hits_arr.size(); jj++){
                //         if (fabs(global.z() - z_hits_arr[jj]) < 1. ){
                //             hresiduals_vollaybins[vollayID]->Fill( global_r - r_hits_arr[jj]);
                //             hresiduals_layers_in_pbins[vollayID][pbin]->Fill( global_r - r_hits_arr[jj]);
                //             // break;
                //         }
                //     }
                // }
            }
        }); //End visiting track points

        if(m_measurementChi2.size() != m_nCalibrated){
            cout << "meas size " << m_measurementChi2.size() << " and # states " << m_nCalibrated << endl;
            test_counter++;
        }
     
        m_log->trace("Number of calibrated states: {}",m_nCalibrated);
        num_traj++;
        
        //Fill histograms
        if(num_primary==1){
            h1a->Fill(mcp,p_traj);
            hchi2->Fill(m_chi2Sum);
            hNDF->Fill(m_NDF);
            hchi2_by_hits->Fill(m_chi2Sum/nHitsallTrackers);
            hchi2_by_NDF->Fill(m_chi2Sum/m_NDF);
            hchi2_by_meas->Fill(m_chi2Sum/m_nMeasurements);
            
            hchi2_vs_eta->Fill(mceta, m_chi2Sum);
            hchi2_vs_hits->Fill(nHitsallTrackers, m_chi2Sum);
            hchi2_vs_hits_zoomed->Fill(nHitsallTrackers, m_chi2Sum);
            heta_vs_p_vs_chi2->Fill(mceta, mcp, m_chi2Sum);

            hNDF_states->Fill(m_NDF,state_counter);
            
            hmeasptrack_vs_eta->Fill(mceta, m_nMeasurements);
            hmeasptrack_vs_hits->Fill(nHitsallTrackers, m_nMeasurements);
            hmeasptrack_vs_chi2perNDF->Fill(m_chi2Sum/m_NDF, m_nMeasurements);
            hmeasptrack_vs_calstates->Fill(m_nCalibrated, m_nMeasurements);
            
            //floor(2*eta + 8) should give the index where bounds are [beg,end)
            int index = floor(2*mceta+8);
            hchi2_vs_hits_etabins[index]->Fill(nHitsallTrackers, m_chi2Sum);
            hmeasptrack_vs_hits_etabins[index]->Fill(nHitsallTrackers, m_nMeasurements);
            hmeasptrack_vs_hits_etabins_zoomed[index]->Fill(nHitsallTrackers, m_nMeasurements);
            hmeasptrack_vs_chi2perNDF_etabins[index]->Fill(m_chi2Sum/m_NDF, m_nMeasurements);
            

            for (int j=0; j<m_measurementChi2.size(); j++){
                hmeaschi2_vs_chi2->Fill(m_chi2Sum, m_measurementChi2[j]);
                hmeaschi2_vs_eta->Fill(mceta, m_measurementChi2[j]);
                hmeaschi2_vs_hits->Fill(nHitsallTrackers, m_measurementChi2[j]);
            }

            hholes_vs_hits->Fill(nHitsallTrackers, m_nHoles);
            houtliers_vs_hits->Fill(nHitsallTrackers, m_nOutliers);
            hsummation->Fill(m_nMeasurements + m_nOutliers, m_nCalibrated);
            hsummation2->Fill(m_nOutliers + m_nMeasurements, nHitsallTrackers);
            hsummation3->Fill(m_nMeasurements + m_nOutliers + m_nHoles, m_nCalibrated);
            //is m_nStates == state_counter??   
            
        }

    } //End loop over trajectories
    printf("for loop finished\n");

    if(num_primary==1){
        heta->Fill(mceta);
        hp->Fill(mcp);
        hpt->Fill(mcpt);
        hhits->Fill(nHitsallTrackers);
        htracks_vs_eta->Fill(mceta, num_traj);

        hhits_vs_eta->Fill(mceta, nHitsallTrackers);
        if(num_traj>0) hhits_vs_eta_1->Fill(mceta, nHitsallTrackers);
    }
    
    if (r_measurements_arr.size() == 0 || r_hits_arr.size() == 0) {
        m_log->trace("-------------------------");
        return;
    }
    event_number++;
    total_measurements += r_measurements_arr.size();
    average_number_of_measurements = (float) total_measurements / (float) event_number;
    printf("Running total of measurements: %d\n", total_measurements);
    printf("Running average of number of measurements %f\n", average_number_of_measurements);
    sort(r_hits_arr.begin(), r_hits_arr.end());
    sort(z_hits_arr.begin(), z_hits_arr.end());
    bool* r_taken = new bool[r_hits_arr.size()];
    bool* z_taken = new bool[z_hits_arr.size()];
    for (int i = 0; i < r_hits_arr.size(); i++) {
        r_taken[i] = false;
    }
    for (int i = 0; i < z_hits_arr.size(); i++) {
        z_taken[i] = false;
    }
    printf("check1\n");
    //now it works  
    //r_matches is an array that matches r_measurements to hit
    int* r_matches = new int[r_measurements_arr.size()];
    //z_matches is an array that matches r_measurements to hit
    int* z_matches = new int[z_measurements_arr.size()];
    printf("check2\n");
    //matches is an array that has the index of the value to match it to
    float prev_diff = fabs(r_measurements_arr[0] - r_hits_arr[0]); 
    float curr_diff;
    for (int i = 0; i < r_measurements_arr.size(); i++) {
        for (int j = 0; j < r_hits_arr.size(); j++) {
            curr_diff = fabs(r_measurements_arr[i] - r_hits_arr[j]);
            if ((curr_diff < prev_diff && (!r_taken[j] || r_hits_arr.size() < r_measurements_arr.size())) || j == 0) {  
                r_matches[i] = j;
                prev_diff = curr_diff;
            }
        }
        r_taken[r_matches[i]] = true;
        if (i < r_measurements_arr.size() - 1) prev_diff = fabs(r_measurements_arr[i+1] - r_hits_arr[0]);
    }
    prev_diff = fabs(z_measurements_arr[0] - z_hits_arr[0]);
    for (int i = 0; i < z_measurements_arr.size(); i++) {
        for (int j = 0; j < z_hits_arr.size(); j++) {
            curr_diff = fabs(z_measurements_arr[i] - z_hits_arr[j]);
            if ((curr_diff < prev_diff && (!z_taken[j] || z_hits_arr.size() < z_measurements_arr.size())) || j == 0) {
                z_matches[i] = j;
                prev_diff = curr_diff;
            }
        }
        z_taken[z_matches[i]] = true;
        if (i < z_measurements_arr.size() - 1) prev_diff = fabs(z_measurements_arr[i+1] - z_hits_arr[0]);
    }
    printf("# of rs: %d\n", r_measurements_arr.size());
    printf("# of r hits: %d\n", r_hits_arr.size());
    for (int i = 0; i < r_measurements_arr.size(); i++) {
        printf("r measurement: %f\n", r_measurements_arr[i]);
        printf("r match: %d\n", r_matches[i]);
    }
    printf("# of zs: %d\n", z_measurements_arr.size());
    printf("# of z hits: %d\n", z_hits_arr.size());
    for (int i = 0; i < z_measurements_arr.size(); i++) {
        printf("z measurement: %f\n", z_measurements_arr[i]);
        printf("z match: %d\n", z_matches[i]);      
    }
    //Calculate Residuals
    for (int i = 0; i < r_measurements_arr.size(); i++) {
        if (indexregsaved == 1) hresiduals[indexregsaved]->Fill(z_measurements_arr[i] - z_hits_arr[z_matches[i]]);
        printf("Vollay ID[i] for r = %d\n", vollayids[i]);
        printf("pbins[i] for r = %d\n", pbins[i]);
        hresiduals_vollaybins[vollayids[i]]->Fill(z_measurements_arr[i] - z_hits_arr[z_matches[i]]);
        hresiduals_layers_in_pbins[vollayids[i]][pbins[i]]->Fill(z_measurements_arr[i] - z_hits_arr[z_matches[i]]);
    }
    for (int i = 0; i < z_measurements_arr.size(); i++) {
        printf("Vollay ID[i] for z = %d\n", vollayids[i]);
        printf("pbins[i] for z = %d\n", pbins[i]);
        if (indexregsaved != 1) hresiduals[indexregsaved]->Fill(r_measurements_arr[i] - r_hits_arr[r_matches[i]]);
        hresiduals_vollaybins[vollayids[i]]->Fill(r_measurements_arr[i] - r_hits_arr[r_matches[i]]);
        hresiduals_layers_in_pbins[vollayids[i]][pbins[i]]->Fill(r_measurements_arr[i] - r_hits_arr[r_matches[i]]);
    }
    m_log->trace("-------------------------");
    printf("final check");
}
 
//------------------
// Finish
//------------------
void trackqa_processor::Finish()
{
    printf("is it finish");
    cout << "The number of times the state counter and meas chi^2 are diff is " << test_counter <<"/10000" << endl;

	m_log->trace("trackqa_processor finished\n");
}
