#include <fstream>
#include <map>
#include "TROOT.h"
#include "TClass.h"
#include "TFile.h"
#include "TLine.h"
#include "TTree.h"
#include "TBrowser.h"
#include "TH2.h"
#include "TH1.h"
#include "TProfile.h"
#include "TRandom.h"
#include <vector>
#include "TVector3.h"
#include "TLorentzVector.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TTreeReaderArray.h"
#include "TLatex.h"

#include "DD4hep/Detector.h"
#include "DD4hep/DetElement.h"

#include "Acts/Plugins/DD4hep/ConvertDD4hepDetector.hpp"
#include "Acts/Plugins/DD4hep/DD4hepFieldAdapter.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Plugins/DD4hep/ConvertDD4hepDetector.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Utilities/BinningType.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Vertexing/ImpactPointEstimator.hpp"

using namespace std;

const double gPionMass = 0.13957;
const double gKaonMass = 0.493677;
const double gprotonMass = 0.938270281;

void anasecvertex(TString listname = "file.list", const TString outname = "test.root"){
  TChain *chain = new TChain("events");

  int nfiles = 0;
  char filename[512];
  ifstream *inputstream = new ifstream;
  inputstream->open(listname.Data());
  if(!inputstream){
    printf("[e] Cannot open file list: %s\n", listname.Data());
  }
  while(inputstream->good()){
    inputstream->getline(filename, 512);
    if(inputstream->good()){
      TFile *ftmp = TFile::Open(filename, "read");
      if(!ftmp||!(ftmp->IsOpen())||!(ftmp->GetNkeys())){
        printf("[e] Could you open file: %s\n", filename);
      }else{
        cout<<"[i] Add "<<nfiles<<"th file: "<<filename<<endl;
        chain->Add(filename);
        nfiles++;
      }
    }
  }
  inputstream->close();
  printf("[i] Read in %d files with %lld events in total\n", nfiles, chain->GetEntries());

  //-------- Do Lambda analysis
  TH1F *hEventStat = new TH1F("hEventStat", "Event statistics", 5, 0, 5);
  hEventStat->GetXaxis()->SetBinLabel(1, "MC events");
  hEventStat->GetXaxis()->SetBinLabel(2, "#Lambda");
  hEventStat->GetXaxis()->SetBinLabel(3, "#Lambda -> p#pi");
  hEventStat->GetXaxis()->SetBinLabel(4, "Reco #Lambda");

  TH1F *hMcVtxX = new TH1F("hMcVtxX", "x position of MC vertex;x (mm)", 100, -5.05, 4.95);
  TH1F *hMcVtxY = new TH1F("hMcVtxY", "y position of MC vertex;y (mm)", 500, -5.01, 4.99);
  TH1F *hMcVtxZ = new TH1F("hMcVtxZ", "z position of MC vertex;z (mm)", 400, -200, 200);

  TH2F *hLambdaDecayVxVy = new TH2F("hLambdaDecayVxVy", "#Lambda decay vertex to primary vertex;#Deltav_{x} (mm);#Deltav_{y} (mm)", 400, -1-0.0025, 1-0.0025, 400, -1-0.0025, 1-0.0025);
  TH2F *hLambdaDecayVrVz = new TH2F("hLambdaDecayVrVz", "#Lambda decay vertex to primary vertex;#Deltav_{z} (mm);#Deltav_{r} (mm)", 100, -2, 2, 100, -0.2, 1.8);

  TH2F *hLambdaDecayVxVyReco = new TH2F("hLambdaDecayVxVyReco", "Reconstructed #Lambda decay vertex to primary vertex;#Deltav_{x} (mm);#Deltav_{y} (mm)", 400, -1-0.0025, 1-0.0025, 400, -1-0.0025, 1-0.0025);
  TH2F *hLambdaDecayVrVzReco = new TH2F("hLambdaDecayVrVzReco", "Reconstructed #Lambda decay vertex to primary vertex;#Deltav_{z} (mm);#Deltav_{r} (mm)", 100, -2, 2, 100, -0.2, 1.8);

  TH2F *hMCLambdaPtRap = new TH2F("hMCLambdaPtRap", "MC D^{0};y;p_{T} (GeV/c)", 20, -5, 5, 100, 0, 10);
  TH2F *hMCLambdaPtRapReco = new TH2F("hMCLambdaPtRapReco", "Reconstructed D^{0} decay vertex;y;p_{T} (GeV/c)", 20, -5, 5, 100, 0, 10);
  TH2F *hLambdaVtxDist = new TH2F("hLambdaVtxDist", "Reconstructed D^{0} vertex to true D^{0} vertex;#Deltav_{z} (mm);#Deltav_{r} (mm)", 100, -1, 1, 100, -1, 1);

  TH2F *hMcPiPtEta = new TH2F("hMcPiPtEta", "MC #pi from #Lambda decay;#eta^{MC};p_{T}^{MC} (GeV/c)", 20, -5, 5, 100, 0, 10);
  TH2F *hMcPiPtEtaReco = new TH2F("hMcPiPtEtaReco", "RC #pi from #Lambda decay;#eta^{MC};p_{T}^{MC} (GeV/c)", 20, -5, 5, 100, 0, 10);

  TH2F *hMcProtonEta = new TH2F("hMcProtonEta", "MC proton from #Lambda decay;#eta^{MC};p_{T}^{MC} (GeV/c)", 20, -5, 5, 100, 0, 10);
  TH2F *hMcProtonEtaReco = new TH2F("hMcProtonEtaReco", "RC proton from #Lambda decay;#eta^{MC};p_{T}^{MC} (GeV/c)", 20, -5, 5, 100, 0, 10);

  TH1F *hNRecoVtx = new TH1F("hNRecoVtx", "Number of reconstructed vertices;N", 10, 0, 10);

  const char* part_name[2] = {"Pi", "proton"};
  const char* part_title[2] = {"#pi", "proton"};
  TH3F *hRcSecPartLocaToMCVtx[2];
  TH3F *hRcSecPartLocbToMCVtx[2];
  TH3F *hRcPrimPartLocaToMCVtx[2];
  TH3F *hRcPrimPartLocbToMCVtx[2];
  TH3F *hRcSecPartLocaToRCVtx[2];
  TH3F *hRcSecPartLocbToRCVtx[2];
  TH3F *hRcPrimPartLocaToRCVtx[2];
  TH3F *hRcPrimPartLocbToRCVtx[2];
  for(int i=0; i<2; i++){
    hRcSecPartLocaToMCVtx[i] = new TH3F(Form("hRcSec%sLocaToMCVtx",part_name[i]), Form("Loc.a distribution for D^{0} decayed %s;p_{T} (GeV/c);#eta;loc.a (mm)", part_title[i]), 100, 0, 10, 20, -5, 5, 100, 0, 1);
    hRcSecPartLocbToMCVtx[i] = new TH3F(Form("hRcSec%sLocbToMCVtx",part_name[i]), Form( "Loc.b distribution for D^{0} decayed %s;p_{T} (GeV/c);#eta;loc.b (mm)", part_title[i]), 100, 0, 10, 20, -5, 5, 100, -0.5, 0.5);
    hRcPrimPartLocaToMCVtx[i] = new TH3F(Form("hRcPrim%sLocaToMCVtx",part_name[i]), Form( "Loc.a distribution for primary %s;p_{T} (GeV/c);#eta;loc.a (mm)", part_title[i]), 100, 0, 10, 20, -5, 5, 100, 0, 1);
    hRcPrimPartLocbToMCVtx[i] = new TH3F(Form("hRcPrim%sLocbToMCVtx",part_name[i]), Form( "Loc.b distribution for primary %s;p_{T} (GeV/c);#eta;loc.b (mm)", part_title[i]), 100, 0, 10, 20, -5, 5, 100, -0.5, 0.5);

    hRcSecPartLocaToRCVtx[i] = new TH3F(Form("hRcSec%sLocaToRCVtx",part_name[i]), Form( "Loc.a distribution for D^{0} decayed %s;p_{T} (GeV/c);#eta;loc.a (mm)", part_title[i]), 100, 0, 10, 20, -5, 5, 100, 0, 1);
    hRcSecPartLocbToRCVtx[i] = new TH3F(Form("hRcSec%sLocbToRCVtx",part_name[i]), Form( "Loc.b distribution for D^{0} decayed %s;p_{T} (GeV/c);#eta;loc.b (mm)", part_title[i]), 100, 0, 10, 20, -5, 5, 100, -0.5, 0.5);
    hRcPrimPartLocaToRCVtx[i] = new TH3F(Form("hRcPrim%sLocaToRCVtx",part_name[i]), Form( "Loc.a distribution for primary %s;p_{T} (GeV/c);#eta;loc.a (mm)", part_title[i]), 100, 0, 10, 20, -5, 5, 100, 0, 1);
    hRcPrimPartLocbToRCVtx[i] = new TH3F(Form("hRcPrim%sLocbToRCVtx",part_name[i]), Form( "Loc.b distribution for primary %s;p_{T} (GeV/c);#eta;loc.b (mm)", part_title[i]), 100, 0, 10, 20, -5, 5, 100, -0.5, 0.5);
  }

  // Invariant mass
  const char* pair_name[2] = {"all", "DCA"};
  TH3F *h3InvMass[2];
  for(int i=0; i<2; i++){
    h3InvMass[i] = new TH3F(Form("h3InvMass_%s", pair_name[i]), "Invariant mass of unlike-sign #piK pairs;p_{T} (GeV/c);y;M_{#piK} (GeV/c^{2})", 100, 0, 10, 20, -5, 5, 100, 1.6, 2.0);
  }

  // Secondary vertex to primary vertex: x,y,z
  //------Lambda kinematic plots
  TH2D*h2LambdapTvy=new TH2D("h2LambdapTvy","#Lambda P_{T} Vs y ",100,-2,4,100,0,10);
  h2LambdapTvy->GetXaxis()->SetTitle("rapidity y");
  h2LambdapTvy->GetYaxis()->SetTitle("P_{T} [GeV]"); 

  TH2D*h2Lambdapveta=new TH2D("h2Lambdapveta","#Lambda p Vs #eta",100,0,10,100,0,20); 
  h2Lambdapveta->GetXaxis()->SetTitle("pseudorapidity #eta"); 
  h2Lambdapveta->GetYaxis()->SetTitle("P [GeV]");

  TH1D* h1MassL=new TH1D("h1MassL","",100,0.9,1.5);
  h1MassL->GetXaxis()->SetTitle("Sec. Vertex Mass [GeV]");

  TH1* h1secX= new TH1D("h1secX","",100,-7.5,7.5);
  h1secX->GetXaxis()->SetTitle("x_{sec.vtx}-x_{prm.vtx} [mm]");
  h1secX->GetXaxis()->CenterTitle();
  h1secX->SetLineWidth(2);

  TH1* h1secY= new TH1D("h1secY","",100,-5.5,5.5);
  h1secY->GetXaxis()->SetTitle("y_{sec.vtx}-y_{prm.vtx} [mm]");
  h1secY->GetXaxis()->CenterTitle();
  h1secY->SetLineWidth(2);

  TH1* h1secZ= new TH1D("h1secZ","",400,-200.,200.);
  h1secZ->GetXaxis()->SetTitle(" z_{sec.vtx}-z_{prm.vtx} [mm]");
  h1secZ->GetXaxis()->CenterTitle();
  h1secZ->SetLineWidth(2);

  TH1 *h1R = new TH1D("h1R","Distance R mm ",200,0,200);
  h1R->GetXaxis()->SetTitle("#Delta R (Sec vtx - Prim vtx) [mm]");
  h1R->GetXaxis()->CenterTitle();
  h1R->SetLineWidth(2);

  TH1 *h1mcR = new TH1D("h1mcR","Distance (MC) R mm ",200,0,200);
  h1mcR->GetXaxis()->SetTitle("#Delta R (Sec vtx - MC Prim vtx) [mm]");
  h1mcR->GetXaxis()->CenterTitle();
  h1mcR->SetLineWidth(2);

  TH1 *h1secvtxNum = new TH1D("h1secvtxNum","",15,0,60);
  h1secvtxNum->GetXaxis()->SetTitle("No. of Two-Track Vertices");
  h1secvtxNum->GetXaxis()->CenterTitle();
  h1secvtxNum->SetLineWidth(2);

  TH1 *h1DistVBarr = new TH1D("h1DistVBarr","Distance R mm ",140,0,70);
  h1DistVBarr->GetXaxis()->SetTitle("SV distance to Vertex Barrel [mm]");
  h1DistVBarr->GetXaxis()->CenterTitle();
  h1DistVBarr->SetLineWidth(2);

  TH1 *h1DistSiBarr = new TH1D("h1DistSciBarr","Distance R mm ",180,20,100);
  h1DistSiBarr->GetXaxis()->SetTitle("SV distance to Si Barrel [mm]");
  h1DistSiBarr->GetXaxis()->CenterTitle();
  h1DistSiBarr->SetLineWidth(2);

  // Define variables
  int pid_code = 13;
  std::string coll = "CentralCKFTrackParameters"; //Real-seeded track parameters
  std::string pvert="CentralTrackVertices";
  std::string svert="AMVSecondaryTrackVertices";
  std::string recoprt="ReconstructedChargedParticles";
  std::string sibarrelvtx="SiBarrelVertexRecHits";
  std::string vtxbarrel="VertexBarrelHits";

  TTreeReader treereader(chain);
  // MC
  TTreeReaderArray<int> mcPartGenStatus = {treereader, "MCParticles.generatorStatus"};
  TTreeReaderArray<int> mcPartPdg = {treereader, "MCParticles.PDG"};
  TTreeReaderArray<float> mcPartCharge = {treereader, "MCParticles.charge"};
  TTreeReaderArray<unsigned int> mcPartParent_begin = {treereader, "MCParticles.parents_begin"};
  TTreeReaderArray<unsigned int> mcPartParent_end = {treereader, "MCParticles.parents_end"};
  TTreeReaderArray<int> mcPartParent_index = {treereader, "_MCParticles_parents.index"};
  TTreeReaderArray<unsigned int> mcPartDaughter_begin = {treereader, "MCParticles.daughters_begin"};
  TTreeReaderArray<unsigned int> mcPartDaughter_end = {treereader, "MCParticles.daughters_end"};
  TTreeReaderArray<int> mcPartDaughter_index = {treereader, "_MCParticles_daughters.index"};
  TTreeReaderArray<double> mcPartMass = {treereader, "MCParticles.mass"};
  TTreeReaderArray<double> mcPartVx = {treereader, "MCParticles.vertex.x"};
  TTreeReaderArray<double> mcPartVy = {treereader, "MCParticles.vertex.y"};
  TTreeReaderArray<double> mcPartVz = {treereader, "MCParticles.vertex.z"};
  TTreeReaderArray<float> mcMomPx = {treereader, "MCParticles.momentum.x"};
  TTreeReaderArray<float> mcMomPy = {treereader, "MCParticles.momentum.y"};
  TTreeReaderArray<float> mcMomPz = {treereader, "MCParticles.momentum.z"};
  TTreeReaderArray<double> mcEndPointX = {treereader, "MCParticles.endpoint.x"};
  TTreeReaderArray<double> mcEndPointY = {treereader, "MCParticles.endpoint.y"};
  TTreeReaderArray<double> mcEndPointZ = {treereader, "MCParticles.endpoint.z"};

  //----------------- Reconstructed Particles ------------------
  // ********** Reco Charged Particles
  TTreeReaderArray<unsigned int> assocChSimID = {treereader, "ReconstructedChargedParticleAssociations.simID"};
  TTreeReaderArray<unsigned int> assocChRecID = {treereader, "ReconstructedChargedParticleAssociations.recID"};
  
  TTreeReaderArray<float> rcMomPx = {treereader, "ReconstructedChargedParticles.momentum.x"};
  TTreeReaderArray<float> rcMomPy = {treereader, "ReconstructedChargedParticles.momentum.y"};
  TTreeReaderArray<float> rcMomPz = {treereader, "ReconstructedChargedParticles.momentum.z"};

  TTreeReaderArray<float> rcTrkLoca = {treereader, "CentralCKFTrackParameters.loc.a"};
  TTreeReaderArray<float> rcTrkLocb = {treereader, "CentralCKFTrackParameters.loc.b"};
  TTreeReaderArray<float> rcTrkqOverP = {treereader, "CentralCKFTrackParameters.qOverP"};
  TTreeReaderArray<float> rcTrkTheta = {treereader, "CentralCKFTrackParameters.theta"};
  TTreeReaderArray<float> rcTrkPhi = {treereader, "CentralCKFTrackParameters.phi"};
  // ******* Primary Vertex
  TTreeReaderArray<float> CTVx = {treereader, "CentralTrackVertices.position.x"};
  TTreeReaderArray<float> CTVy = {treereader, "CentralTrackVertices.position.y"};
  TTreeReaderArray<float> CTVz = {treereader, "CentralTrackVertices.position.z"};
  TTreeReaderArray<int> CTVndf = {treereader, "CentralTrackVertices.ndf"};
  TTreeReaderArray<float> CTVchi2 = {treereader, "CentralTrackVertices.chi2"};
  TTreeReaderArray<float> CTVerr_xx = {treereader, "CentralTrackVertices.positionError.xx"};
  TTreeReaderArray<float> CTVerr_yy = {treereader, "CentralTrackVertices.positionError.yy"};
  TTreeReaderArray<float> CTVerr_zz = {treereader, "CentralTrackVertices.positionError.zz"};
  // ******* Secondary Vertex
  TTreeReaderArray<float> svert_chi2(treereader, Form("%s.chi2",svert.c_str()));
  TTreeReaderArray<float> svert_vx(treereader, Form("%s.position.x",svert.c_str()));
  TTreeReaderArray<float> svert_vy(treereader, Form("%s.position.y",svert.c_str()));
  TTreeReaderArray<float> svert_vz(treereader, Form("%s.position.z",svert.c_str()));
  TTreeReaderArray<unsigned int> svertAssoc(treereader, Form("%s.associatedParticles_begin",svert.c_str()));

  TTreeReaderArray<float> track_qoverp(treereader, Form("%s.qOverP",coll.c_str()));
  TTreeReaderArray<float> track_theta(treereader, Form("%s.theta",coll.c_str()));
  TTreeReaderArray<float> track_phi(treereader, Form("%s.phi",coll.c_str()));
  TTreeReaderArray<float> track_loca(treereader, Form("%s.loc.a",coll.c_str()));
  TTreeReaderArray<float> track_locb(treereader, Form("%s.loc.b",coll.c_str()));
  TTreeReaderArray<float> track_time(treereader, Form("%s.time",coll.c_str()));
  //Reconstructed Charged Particle
  TTreeReaderArray<int>   prt_pid(treereader,Form("%s.PDG",recoprt.c_str()));
  TTreeReaderArray<float> prt_px(treereader,Form("%s.momentum.x",recoprt.c_str()));
  TTreeReaderArray<float> prt_py(treereader,Form("%s.momentum.y",recoprt.c_str()));
  TTreeReaderArray<float> prt_pz(treereader,Form("%s.momentum.z",recoprt.c_str()));
  TTreeReaderArray<float> prt_M(treereader,Form("%s.mass",recoprt.c_str()));
  TTreeReaderArray<float> prt_charge(treereader,Form("%s.charge",recoprt.c_str()));

  TTreeReaderArray<int> prim_vtx_index = {treereader, "PrimaryVertices_objIdx.index"};

  TTreeReaderArray<unsigned int> vtxAssocPart_begin = {treereader, "CentralTrackVertices.associatedParticles_begin"};
  TTreeReaderArray<unsigned int> vtxAssocPart_end = {treereader, "CentralTrackVertices.associatedParticles_end"};
  TTreeReaderArray<int> vtxAssocPart_index = {treereader, "_CentralTrackVertices_associatedParticles.index"};

    // Load DD4Hep geometry
  dd4hep::Detector& detector = dd4hep::Detector::getInstance();
  detector.fromCompact("/opt/detector/epic-main/share/epic/epic_craterlake.xml");
  dd4hep::DetElement geometry = detector.world();

  // Convert DD4Hep geometry to tracking geometry
  Acts::GeometryContext trackingGeoCtx;
  auto logger = Acts::getDefaultLogger("DD4hepConversion", Acts::Logging::Level::INFO);
  Acts::BinningType bTypePhi = Acts::equidistant;
  Acts::BinningType bTypeR = Acts::equidistant;
  Acts::BinningType bTypeZ = Acts::equidistant;
  double layerEnvelopeR = Acts::UnitConstants::mm;
  double layerEnvelopeZ = Acts::UnitConstants::mm;
  double defaultLayerThickness = Acts::UnitConstants::fm;
  using Acts::sortDetElementsByID;

  std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry{nullptr};
  trackingGeometry = Acts::convertDD4hepDetector(geometry,*logger,bTypePhi,bTypeR,bTypeZ,layerEnvelopeR,layerEnvelopeZ,defaultLayerThickness,sortDetElementsByID,trackingGeoCtx);

  // Define Perigee surface at which reconstructed track parameters are set
  auto perigee = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(0,0,0));

  // Get Magnetic field context
  Acts::MagneticFieldContext fieldctx;
  std::shared_ptr<const Acts::DD4hepFieldAdapter> field_provider = std::make_shared<const Acts::DD4hepFieldAdapter>(detector.field());
  Acts::MagneticFieldProvider::Cache field_cache = field_provider->makeCache(fieldctx);

  // Stepper and Propagator
  using Stepper    = Acts::EigenStepper<>;
  using Propagator = Acts::Propagator<Stepper>;

  Stepper stepper(field_provider);
  Propagator propagator(stepper);

  // Create Impact Point Estimator
  Acts::ImpactPointEstimator::Config ImPoEs_cfg(field_provider,std::make_shared<Propagator>(propagator));
  Acts::ImpactPointEstimator::State ImPoEs_state;
  ImPoEs_state.fieldCache = field_cache;
  Acts::ImpactPointEstimator ImPoEs(ImPoEs_cfg);

  int nevents = 0;
  while(treereader.Next())
    {
      if(nevents%2==0) printf("\n[i] New event %d\n",nevents);
      //if(nevents==20) break;
      //printf("\n+++++ New Event %d +++++\n", nevents);

      // find MC primary vertex
      int nMCPart = mcPartMass.GetSize();
      TVector3 vertex_mc(-999., -999., -999.);
      for(int imc=0; imc<nMCPart; imc++){
        if(mcPartGenStatus[imc] == 4 && mcPartPdg[imc] == 11){
          vertex_mc.SetXYZ(mcEndPointX[imc], mcEndPointY[imc], mcEndPointZ[imc]);
          printf("[i] Primary vertex (x, y, z) = (%2.4f, %2.4f, %2.4f)\n", vertex_mc.x(), vertex_mc.y(), vertex_mc.z());
          break;
        }
      }

      // get RC primary vertex
      TVector3 vertex_rc(-999., -999., -999.);
      if(prim_vtx_index.GetSize()>0){
        int rc_vtx_index = prim_vtx_index[0];
        vertex_rc.SetXYZ(CTVx[rc_vtx_index], CTVy[rc_vtx_index], CTVz[rc_vtx_index]);
      }
      
      int nAssoc = assocChRecID.GetSize();
      map<int, int> assoc_map;

      for(int j=0; j<nAssoc; j++){
        assoc_map[assocChSimID[j]] = assocChRecID[j];
      }

      bool hasLambda = false;
      for(int imc=0; imc<nMCPart; imc++){
        // loop over primary particles
        if(mcPartGenStatus[imc] == 1 && mcPartCharge[imc] != 0){
          double dist=sqrt(pow(mcPartVx[imc]-vertex_mc.x(),2) + pow(mcPartVy[imc]-vertex_mc.y(),2) + pow(mcPartVz[imc]-vertex_mc.z(),2));      
          if(dist < 1e-4){
            // check if the MC particle is reconstructed
            int rc_index = -1;
            if(assoc_map.find(imc) != assoc_map.end()) rc_index = assoc_map[imc];
            if(rc_index>-1){
              TVector3 rc_vec(rcMomPx[rc_index], rcMomPy[rc_index], rcMomPz[rc_index]);
            
              Acts::BoundVector params;
              params(Acts::eBoundLoc0)   = rcTrkLoca[rc_index];
              params(Acts::eBoundLoc1)   = rcTrkLocb[rc_index];
              params(Acts::eBoundPhi)    = rcTrkPhi[rc_index];
              params(Acts::eBoundTheta)  = rcTrkTheta[rc_index];
              params(Acts::eBoundQOverP) = rcTrkqOverP[rc_index];
              params(Acts::eBoundTime)   = 0;
            
              //FIXME: Set covariance matrix based on input ROOT file information
              Acts::BoundSquareMatrix cov = Acts::BoundSquareMatrix::Zero();
            
              Acts::Vector3 mc_vtx_pos(vertex_mc.x() * Acts::UnitConstants::mm, vertex_mc.y() * Acts::UnitConstants::mm, vertex_mc.z() * Acts::UnitConstants::mm);
            
              Acts::Vector3 rc_vtx_pos(vertex_rc.x() * Acts::UnitConstants::mm, vertex_rc.y() * Acts::UnitConstants::mm, vertex_rc.z() * Acts::UnitConstants::mm);
              
              if(mcPartPdg[imc] == -211 || mcPartPdg[imc] == 2212){
                // Acts::ParticleHypothesis particle_hypothesis;
              
                int ip = 0;
                if(mcPartPdg[imc] == 2212) ip = 1;
              
                Acts::BoundTrackParameters track_parameters(perigee,params,cov,Acts::ParticleHypothesis::pion());
                if(mcPartPdg[imc] == 2212) track_parameters = Acts::BoundTrackParameters(perigee,params,cov,Acts::ParticleHypothesis::proton());
              
                //--- Get track parameters at 3D DCA to MC primary vertex ----
                auto result = ImPoEs.estimate3DImpactParameters(trackingGeoCtx,fieldctx,track_parameters,mc_vtx_pos,ImPoEs_state);
                if(result.ok()){
                  Acts::BoundTrackParameters trk_boundpar_vtx = result.value();
                  const auto& trk_vtx_params  = trk_boundpar_vtx.parameters();
                  auto trk_vtx_gbl_pos = trk_boundpar_vtx.position(trackingGeoCtx);
                  //cout << "real: " << trk_vtx_params[Acts::eBoundLoc0] << "  " << trk_vtx_params[Acts::eBoundLoc1] << endl;
                
                  double dca_xy = sqrt( pow(trk_vtx_gbl_pos.x()-mc_vtx_pos.x(),2) + pow(trk_vtx_gbl_pos.y()-mc_vtx_pos.y(),2) );
                  double dca_z = trk_vtx_gbl_pos.z()-mc_vtx_pos.z();
                
                  hRcPrimPartLocaToMCVtx[ip]->Fill(rc_vec.Pt(), rc_vec.Eta(), dca_xy);
                  hRcPrimPartLocbToMCVtx[ip]->Fill(rc_vec.Pt(), rc_vec.Eta(), dca_z);
                }
              
                //--- Get track parameters at 3D DCA to RC primary vertex ----
                if(prim_vtx_index.GetSize()>0){
                  auto result = ImPoEs.estimate3DImpactParameters(trackingGeoCtx,fieldctx,track_parameters,rc_vtx_pos,ImPoEs_state);
                  if(result.ok()){
                    Acts::BoundTrackParameters trk_boundpar_vtx = result.value();
                    const auto& trk_vtx_params  = trk_boundpar_vtx.parameters();
                    auto trk_vtx_gbl_pos = trk_boundpar_vtx.position(trackingGeoCtx);
                    double dca_xy = sqrt( pow(trk_vtx_gbl_pos.x()-rc_vtx_pos.x(),2) + pow(trk_vtx_gbl_pos.y()-rc_vtx_pos.y(),2) );
                    double dca_z = trk_vtx_gbl_pos.z()-rc_vtx_pos.z();
                    hRcPrimPartLocaToRCVtx[ip]->Fill(rc_vec.Pt(), rc_vec.Eta(), dca_xy);
                    hRcPrimPartLocbToRCVtx[ip]->Fill(rc_vec.Pt(), rc_vec.Eta(), dca_z);
                  }
                }
              }
            }
          }
        }
      
        // Lambda
        if(mcPartPdg[imc] == 3122){
          hEventStat->Fill(1.5);
          int nDuaghters = mcPartDaughter_end[imc]-mcPartDaughter_begin[imc];
          if(nDuaghters!=2) continue;
        
          // find Lambda that decay into pi+K
          bool is_piproton_decay = false;
          
          int daug_index_1 = mcPartDaughter_index[mcPartDaughter_begin[imc]];
          int daug_index_2 = mcPartDaughter_index[mcPartDaughter_begin[imc]+1];
          int daug_pdg_1 = mcPartPdg[daug_index_1];
          int daug_pdg_2 = mcPartPdg[daug_index_2];
          if((daug_pdg_1==2212 && daug_pdg_2==-211) || (daug_pdg_1==-211 && daug_pdg_2==2212)){
            is_piproton_decay = true;
          }
          if(!is_piproton_decay) continue;
          hasLambda = true;
          hEventStat->Fill(2.5);
        
          // Lambda kinematics
          TLorentzVector mc_mom_vec;
          mc_mom_vec.SetXYZM(mcMomPx[imc], mcMomPy[imc], mcMomPz[imc], mcPartMass[imc]);
          double mcRap = mc_mom_vec.Rapidity();
          double mcPt = mc_mom_vec.Pt();
          hMCLambdaPtRap->Fill(mcRap, mcPt);
        
          // decay dauther kinematics
          int daug_pi_index = daug_pdg_1==-211 ? daug_index_1 : daug_index_2;
          int daug_proton_index  = daug_pdg_1==2212 ? daug_index_1 : daug_index_2;
          for(int ip = 0; ip<2; ip++){
            int mc_part_index;
            if(ip==0) mc_part_index = daug_pi_index;
            if(ip==1) mc_part_index = daug_proton_index;
          
            TLorentzVector mc_part_vec;
            mc_part_vec.SetXYZM(mcMomPx[mc_part_index], mcMomPy[mc_part_index], mcMomPz[mc_part_index], mcPartMass[mc_part_index]);
            if(ip==0) hMcPiPtEta->Fill(mc_part_vec.Eta(), mc_part_vec.Pt());
            if(ip==1) hMcProtonEta->Fill(mc_part_vec.Eta(), mc_part_vec.Pt());
          
            int rc_part_index = -1;
            if(assoc_map.find(mc_part_index) != assoc_map.end()) rc_part_index = assoc_map[mc_part_index];
          
            if(rc_part_index>=0){
              if(ip==0) hMcPiPtEtaReco->Fill(mc_part_vec.Eta(), mc_part_vec.Pt());
              if(ip==1) hMcProtonEtaReco->Fill(mc_part_vec.Eta(), mc_part_vec.Pt());
            
              TVector3 rc_vec(rcMomPx[rc_part_index], rcMomPy[rc_part_index], rcMomPz[rc_part_index]);
              
              Acts::BoundVector params;
              params(Acts::eBoundLoc0)   = rcTrkLoca[rc_part_index];
              params(Acts::eBoundLoc1)   = rcTrkLocb[rc_part_index];
              params(Acts::eBoundPhi)    = rcTrkPhi[rc_part_index];
              params(Acts::eBoundTheta)  = rcTrkTheta[rc_part_index];
              params(Acts::eBoundQOverP) = rcTrkqOverP[rc_part_index];
              params(Acts::eBoundTime)   = 0;
            
              //FIXME: Set covariance matrix based on input ROOT file information
              Acts::BoundSquareMatrix cov = Acts::BoundSquareMatrix::Zero();
            
              Acts::Vector3 mc_vtx_pos(vertex_mc.x() * Acts::UnitConstants::mm, vertex_mc.y() * Acts::UnitConstants::mm, vertex_mc.z() * Acts::UnitConstants::mm);
            
              Acts::Vector3 rc_vtx_pos(vertex_rc.x() * Acts::UnitConstants::mm, vertex_rc.y() * Acts::UnitConstants::mm, vertex_rc.z() * Acts::UnitConstants::mm);
            
              
              Acts::BoundTrackParameters track_parameters(perigee,params,cov,Acts::ParticleHypothesis::pion());
              if(ip==1) track_parameters = Acts::BoundTrackParameters(perigee,params,cov,Acts::ParticleHypothesis::proton());
            
              //--- Get track parameters at 3D DCA to MC primary vertex ----
              auto result = ImPoEs.estimate3DImpactParameters(trackingGeoCtx,fieldctx,track_parameters,mc_vtx_pos,ImPoEs_state);
              if(result.ok()){
                Acts::BoundTrackParameters trk_boundpar_vtx = result.value();
                const auto& trk_vtx_params  = trk_boundpar_vtx.parameters();
                auto trk_vtx_gbl_pos = trk_boundpar_vtx.position(trackingGeoCtx);
                double dca_xy = sqrt( pow(trk_vtx_gbl_pos.x()-mc_vtx_pos.x(),2) + pow(trk_vtx_gbl_pos.y()-mc_vtx_pos.y(),2) );
                double dca_z = trk_vtx_gbl_pos.z()-mc_vtx_pos.z();
                hRcSecPartLocaToMCVtx[ip]->Fill(rc_vec.Pt(), rc_vec.Eta(), dca_xy);
                hRcSecPartLocbToMCVtx[ip]->Fill(rc_vec.Pt(), rc_vec.Eta(), dca_z);
              }
              
              //--- Get track parameters at 3D DCA to RC primary vertex ----
              if(prim_vtx_index.GetSize()>0){
                auto result = ImPoEs.estimate3DImpactParameters(trackingGeoCtx,fieldctx,track_parameters,rc_vtx_pos,ImPoEs_state);
                if(result.ok()){
                  Acts::BoundTrackParameters trk_boundpar_vtx = result.value();
                  const auto& trk_vtx_params  = trk_boundpar_vtx.parameters();
                  auto trk_vtx_gbl_pos = trk_boundpar_vtx.position(trackingGeoCtx);
                  double dca_xy = sqrt( pow(trk_vtx_gbl_pos.x()-rc_vtx_pos.x(),2) + pow(trk_vtx_gbl_pos.y()-rc_vtx_pos.y(),2) );
                  double dca_z = trk_vtx_gbl_pos.z()-rc_vtx_pos.z();
                  hRcSecPartLocaToRCVtx[ip]->Fill(rc_vec.Pt(), rc_vec.Eta(), dca_xy);
                  hRcSecPartLocbToRCVtx[ip]->Fill(rc_vec.Pt(), rc_vec.Eta(), dca_z);
                }
              }
            }
          }
        
          // decay vertex
          TVector3 mc_vtx_decay(-999.,-999.,-999.);
          mc_vtx_decay.SetXYZ(mcEndPointX[imc], mcEndPointY[imc], mcEndPointZ[imc]);
          double mc_decay_dvx = mc_vtx_decay.x()-vertex_mc.x();
          double mc_decay_dvy = mc_vtx_decay.y()-vertex_mc.y();
          double mc_decay_dvz = mc_vtx_decay.z()-vertex_mc.z();
          double mc_decay_dvr = mc_decay_dvx*mc_decay_dvx + mc_decay_dvy*mc_decay_dvy;
          hLambdaDecayVxVy->Fill(mc_decay_dvx, mc_decay_dvy);
          hLambdaDecayVrVz->Fill(mc_decay_dvz, mc_decay_dvr);
        
          //printf("[i] Found Lambda decay at (%2.4f, %2.4f, %2.4f)\n", mc_vtx_decay.x(), mc_vtx_decay.y(), mc_vtx_decay.z() );
        
          // check if the decay vertex is reconstructed
          TVector3 rc_vtx_decay(-999, -999, -999);
          for(unsigned int v=0; v<CTVx.GetSize(); v++){
            if( vtxAssocPart_end[v]-vtxAssocPart_begin[v] != 2) continue;
          
            bool found_lambda = true;
            for(int irc = vtxAssocPart_begin[v]; irc < vtxAssocPart_end[v]; irc++){
              int index = vtxAssocPart_index[irc];
              int iSimPartID = -1;
              for(int j=0; j<nAssoc; j++){
                if(assocChRecID[j]==index){
                  iSimPartID = assocChSimID[j];
                  break;
                }
              }
              if(iSimPartID!=daug_index_1 && iSimPartID!=daug_index_2){
                found_lambda = false;
                break;
              }
            }
            if(found_lambda){
              hMCLambdaPtRapReco->Fill(mcRap, mcPt);
              rc_vtx_decay.SetXYZ(CTVx[v], CTVy[v], CTVz[v]);
              double rc_decay_dvx = rc_vtx_decay.x() - mc_vtx_decay.x();
              double rc_decay_dvy = rc_vtx_decay.y() - mc_vtx_decay.y();
              double rc_decay_dvz = rc_vtx_decay.z() - mc_vtx_decay.z();
              double rc_decay_dvr = rc_decay_dvx*rc_decay_dvx + rc_decay_dvy*rc_decay_dvy;
              hLambdaVtxDist->Fill(rc_decay_dvz, rc_decay_dvr);
              hLambdaDecayVxVyReco->Fill(mc_decay_dvx, mc_decay_dvy);
              hLambdaDecayVrVzReco->Fill(mc_decay_dvz, mc_decay_dvr);
              printf("[i] Reco Lambda decay at (%2.4f, %2.4f, %2.4f)\n", rc_vtx_decay.x(), rc_vtx_decay.y(), rc_vtx_decay.z() );
            }
          }
        }
      }
      hEventStat->Fill(0.5);
      hMcVtxX->Fill(vertex_mc.x());
      hMcVtxY->Fill(vertex_mc.y());
      hMcVtxZ->Fill(vertex_mc.z());

      hNRecoVtx->Fill(CTVx.GetSize());
      h1secvtxNum->Fill(svert_vx.GetSize());

      // loop over reconstructed particles
      const bool select_lambda = true;
      if(!select_lambda) hasLambda = true;
      if(hasLambda && (prim_vtx_index.GetSize()>0 && svertAssoc.GetSize()>0)){
        // find pion and proton based on true pdg
        vector<int> pi_index;
        vector<int> proton_index;
        vector<int> pi_dca_index;
        vector<int> proton_dca_index;
        pi_index.clear();
        proton_index.clear();
        pi_dca_index.clear();
        proton_dca_index.clear();
        for(int rc_index=0; rc_index<rcMomPx.GetSize(); rc_index++){
          int iSimPartID = -1;
          for(int j=0; j<nAssoc; j++){
            if(assocChRecID[j]==rc_index){
              iSimPartID = assocChSimID[j];
              break;
            }
          }
          if(iSimPartID<0) continue;
          if(mcPartPdg[iSimPartID] == -211 || mcPartPdg[iSimPartID] == 2212){
            if(mcPartPdg[iSimPartID] == -211) pi_index.push_back(rc_index);
            if(mcPartPdg[iSimPartID] == 2212) proton_index.push_back(rc_index);
            
            Acts::BoundVector params;
            params(Acts::eBoundLoc0)   = rcTrkLoca[rc_index];
            params(Acts::eBoundLoc1)   = rcTrkLocb[rc_index];
            params(Acts::eBoundPhi)    = rcTrkPhi[rc_index];
            params(Acts::eBoundTheta)  = rcTrkTheta[rc_index];
            params(Acts::eBoundQOverP) = rcTrkqOverP[rc_index];
            params(Acts::eBoundTime)   = 0;
            
            //FIXME: Set covariance matrix based on input ROOT file information
            Acts::BoundSquareMatrix cov = Acts::BoundSquareMatrix::Zero();
            Acts::Vector3 rc_vtx_pos(vertex_rc.x() * Acts::UnitConstants::mm, vertex_rc.y() * Acts::UnitConstants::mm, vertex_rc.z() * Acts::UnitConstants::mm);
            Acts::BoundTrackParameters track_parameters(perigee,params,cov,Acts::ParticleHypothesis::pion());
            if(mcPartPdg[iSimPartID] == 2212) track_parameters = Acts::BoundTrackParameters(perigee,params,cov,Acts::ParticleHypothesis::proton());
          
            auto result = ImPoEs.estimate3DImpactParameters(trackingGeoCtx,fieldctx,track_parameters,rc_vtx_pos,ImPoEs_state);
            if(result.ok()){
              Acts::BoundTrackParameters trk_boundpar_vtx = result.value();
              auto trk_vtx_gbl_pos = trk_boundpar_vtx.position(trackingGeoCtx);
              double dca_xy = sqrt( pow(trk_vtx_gbl_pos.x()-rc_vtx_pos.x(),2) + pow(trk_vtx_gbl_pos.y()-rc_vtx_pos.y(),2) );
              double dca_z = trk_vtx_gbl_pos.z()-rc_vtx_pos.z();
              if(dca_xy>0.04){
                if(mcPartPdg[iSimPartID] == -211) pi_dca_index.push_back(rc_index);
                if(mcPartPdg[iSimPartID] == 2212) proton_dca_index.push_back(rc_index);
              }
            }
          }
        }
      
        // proton pion pair
        for(int i=0; i<pi_index.size(); i++){
          TLorentzVector pi_mom_vec;
          pi_mom_vec.SetXYZM(rcMomPx[pi_index[i]], rcMomPy[pi_index[i]], rcMomPz[pi_index[i]], gPionMass);
          for(int j=0; j<proton_index.size(); j++){
            TLorentzVector proton_mom_vec;
            proton_mom_vec.SetXYZM(rcMomPx[proton_index[j]], rcMomPy[proton_index[j]], rcMomPz[proton_index[j]], gprotonMass);
            //Delta R sec. vtx - reco prim. vtx
            double deltaX=svert_vx[pi_index[i]]-CTVx[pi_index[i]];
            double deltaY=svert_vy[pi_index[i]]-CTVy[pi_index[i]];
            double deltaZ=svert_vz[pi_index[i]]-CTVz[pi_index[i]];
            double deltaR=std::sqrt(deltaX*deltaX+deltaY*deltaY+deltaZ*deltaZ);
            // DecayR MC decay vertex
            TVector3 mc_prmvtx_decay(-999.,-999.,-999.);
            TVector3 sec_vtx_decay(-999.,-999.,-999.);
            for(int mcvtx=0; mcvtx<mcEndPointX.GetSize(); mcvtx++){
              mc_prmvtx_decay.SetXYZ(mcEndPointX[mcvtx], mcEndPointY[mcvtx], mcEndPointZ[mcvtx]);
              sec_vtx_decay.SetXYZ(svert_vx[pi_index[i]], svert_vy[pi_index[i]], svert_vz[pi_index[i]]);
              if(rcTrkqOverP[pi_index[i]]*rcTrkqOverP[proton_index[j]]<0){
                double mc_decay_dx = sec_vtx_decay.x()-mc_prmvtx_decay.x();
                double mc_decay_dy = sec_vtx_decay.y()-mc_prmvtx_decay.y();
                double mc_decay_dz = sec_vtx_decay.z()-mc_prmvtx_decay.z();
                double mc_decay_dr = std::sqrt(mc_decay_dx*mc_decay_dx + mc_decay_dy*mc_decay_dy + mc_decay_dz*mc_decay_dz);
                h1mcR->Fill(deltaR);
              }
            }

            h1secX->Fill(deltaX);
            h1secY->Fill(deltaY);
            h1secZ->Fill(deltaZ);

            if(rcTrkqOverP[pi_index[i]]*rcTrkqOverP[proton_index[j]]<0){
              hEventStat->Fill(3.5);
              TLorentzVector parent = pi_mom_vec + proton_mom_vec;
              h3InvMass[0]->Fill(parent.Pt(), parent.Rapidity(), parent.M());
              h2LambdapTvy->Fill(parent.Pt(),parent.Rapidity());
              h2Lambdapveta->Fill(parent.P(),parent.PseudoRapidity());
              h1MassL->Fill(parent.M());
              h1R->Fill(deltaR);
            }
          }
        }
      
        for(int i=0; i<pi_dca_index.size(); i++){
          TLorentzVector pi_mom_vec;
          pi_mom_vec.SetXYZM(rcMomPx[pi_dca_index[i]], rcMomPy[pi_dca_index[i]], rcMomPz[pi_dca_index[i]], gPionMass);
          for(int j=0; j<proton_dca_index.size(); j++){
            TLorentzVector proton_mom_vec;
            proton_mom_vec.SetXYZM(rcMomPx[proton_dca_index[j]], rcMomPy[proton_dca_index[j]], rcMomPz[proton_dca_index[j]], gprotonMass);
            if(rcTrkqOverP[pi_dca_index[i]]*rcTrkqOverP[proton_dca_index[j]]<0){
              TLorentzVector parent = pi_mom_vec + proton_mom_vec;
              h3InvMass[1]->Fill(parent.Pt(), parent.Rapidity(), parent.M());
            }
          }
        }
      }
	      
      nevents++;
    }

  TFile *outfile = new TFile(outname.Data(), "recreate");

  hEventStat->Write();
  hMcVtxX->Write();
  hMcVtxY->Write();
  hMcVtxZ->Write();

  h1secX->Write();
  h1secY->Write();
  h1secZ->Write();
  h1R->Write();
  h1mcR->Write();
  h1secvtxNum->Write();
  h2LambdapTvy->Write();
  h2Lambdapveta->Write();
  h1MassL->Write();
  
  hLambdaDecayVxVy->Write();
  hLambdaDecayVrVz->Write();

  hLambdaDecayVxVyReco->Write();
  hLambdaDecayVrVzReco->Write();
  
  hMCLambdaPtRap->Write();
  hMCLambdaPtRapReco->Write();
  hLambdaVtxDist->Write();

  hMcPiPtEta->Write();
  hMcPiPtEtaReco->Write();
  hMcProtonEta->Write();
  hMcProtonEtaReco->Write();
  
  hNRecoVtx->Write();

  for(int ip=0; ip<2; ip++)
    {
      hRcSecPartLocaToMCVtx[ip]->Write();
      hRcSecPartLocbToMCVtx[ip]->Write();
      hRcSecPartLocaToRCVtx[ip]->Write();
      hRcSecPartLocbToRCVtx[ip]->Write();

      hRcPrimPartLocaToMCVtx[ip]->Write();
      hRcPrimPartLocbToMCVtx[ip]->Write();
      hRcPrimPartLocaToRCVtx[ip]->Write();
      hRcPrimPartLocbToRCVtx[ip]->Write();
    }

  for(int i=0; i<2; i++)
    {
      h3InvMass[i]->Write();
    }
  
  
  outfile->Close();

}
