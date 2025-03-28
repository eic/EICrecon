// Copyright 2023, Christopher Dilks, adapted from Alexander Kiselev's Juggler implementation `IRTAlgorithm`
// Subject to the terms in the LICENSE file found in the top-level directory.

#include <TFile.h>

#if 1//_TODAY_
#include "IrtDebugging.h"

#include <IRT/ChargedParticle.h>
#if _TODAY_
#include <IRT/CherenkovPID.h>
#include <IRT/OpticalPhoton.h>
#include <IRT/RadiatorHistory.h>
#include <IRT/SinglePDF.h>
#include <TString.h>
#endif
#include <edm4hep/SimTrackerHitCollection.h>
#include <TVector3.h>
#if _TODAY_
#include <edm4eic/CherenkovParticleIDHypothesis.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/TrackPoint.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/Vector2f.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <spdlog/common.h>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <gsl/pointers>
#include <iterator>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

#include "algorithms/pid/IrtCherenkovParticleIDConfig.h"
#endif
#include "algorithms/pid/Tools.h"

#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TBranch.h>
#include "IRT/CherenkovEvent.h"
#include <mutex>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4hep/utils/kinematics.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackSegment.h>

//DreamReadout* DreamReadout::m_Instance = 0;
//thread_local CherenkovEvent *eicrecon::IrtDebugging::m_Event = 0;

// IrtDebugging::process() is const -> move mutex out for the time being;
static std::mutex m_OutputTreeMutex;

//static TFile *m_OutputFile;// = 0;
TFile *eicrecon::IrtDebugging::m_OutputFile = 0;
//    /*static thread_local*/ CherenkovEvent *m_Event;
TTree *eicrecon::IrtDebugging::m_EventTree = 0;
TBranch *eicrecon::IrtDebugging::m_EventBranch = 0;
unsigned eicrecon::IrtDebugging::m_InstanceCounter = 0;
TH1D *eicrecon::IrtDebugging::m_Debug = 0;

static std::map<CherenkovRadiator*, std::string> radiators;

// FIXME: move to a different place;
#define _MAGIC_CFF_                         (1239.8)

#define _FAKE_QE_DOWNSCALING_FACTOR_          (30.0/37.0)

// -------------------------------------------------------------------------------------

//#include "algorithms/digi/PhotoMultiplierHitDigi.h"
#if _OLD_
class LookupTable {
public:
  LookupTable() {};
  ~LookupTable() {};
  
  std::vector<std::pair<double, double>> 
  ApplyFineBinning(const std::vector<std::pair<double, 
		   double>> &input, unsigned nbins);
  
  bool GetFinelyBinnedTableEntry(const std::vector<std::pair<double, double>> &table, 
				 double argument, double *entry) const;
  
  bool QE_pass(double ev, double rand) const;
  
protected:
  std::vector<std::pair<double, double>> m_QE_lookup_table;
};
#endif

class G4DataInterpolation {
public:
  G4DataInterpolation(const double WL[], const double QE[], unsigned dim, double, double) {
    //m_Digi = new eicrecon::PhotoMultiplierHitDigi("qe-functionality-only");

    for(unsigned iq=0; iq<dim; iq++)
      m_RawData.push_back(std::make_pair(WL[iq], QE[iq]));
  };
  ~G4DataInterpolation() {};

  //std::vector<std::pair<double, double>> 
  void CreateLookupTable(unsigned nbins);
  
  bool GetLookupTableEntry(//const std::vector<std::pair<double, double>> &table, 
			   double argument, double *entry) const;
  
  //bool QE_pass(double ev, double rand) const;
  
  std::vector<std::pair<double, double>> m_RawData;
  
  //protected:
  std::vector<std::pair<double, double>> m_LookupTable;
  
  //eicrecon::PhotoMultiplierHitDigi *m_Digi;
#if 0
  double CubicSplineInterpolation(double eph) const {

    return 0.50;
  };
#endif
};

//std::vector<std::pair<double, double>> 
void G4DataInterpolation::CreateLookupTable(unsigned nbins)//const std::vector<std::pair<double, 
//double>> &input, unsigned nbins)
{
  //std::vector<std::pair<double, double>> ret;
  m_LookupTable.clear();
  
  //#if 1//_TODAY_
  // FIXME: could have probably just reordered the initial vector;
  std::map<double, double> buffer;

  for(auto entry: m_RawData)//input)
    buffer[entry.first] = entry.second;

  // Sanity checks; return empty map in case do not pass them;
  if (buffer.size() < 2 || nbins < 2) return;// ret;

  //m_QE_lookup_table.push_back(std::make_pair((*QE_map.begin()).first , 0.3));
  //m_QE_lookup_table.push_back(std::make_pair((*QE_map.rbegin()).first, 0.3)); 

  double from = (*buffer.begin()).first;
  double to   = (*buffer.rbegin()).first;
  // Will be "nbins+1" equidistant entries;
  double step = (to - from) / nbins;

  // Just in case somebody considers to call this method twice :-);
  //m_QE_lookup_table.clear();

  for(auto entry: buffer) {
    double e1 = entry.first;
    double qe1 = entry.second;

    if (!m_LookupTable.size())
      m_LookupTable.push_back(std::make_pair(e1, qe1));
    else {
      const auto &prev = m_LookupTable[m_LookupTable.size()-1];

      double e0 = prev.first;
      double qe0 = prev.second;
      double a = (qe1 - qe0) / (e1 - e0);
      double b = qe0 - a*e0;
      // FIXME: check floating point accuracy when moving to a next point; do we actually 
      // care whether the overall number of bins will be "nbins+1" or more?;
      for(double e = e0+step; e<e1; e+=step)
	m_LookupTable.push_back(std::make_pair(e, a*e + b));
    } //if
  } //for entry
  //#endif
  
  for(auto entry: m_LookupTable) 
    printf("@L@ %7.2f -> %7.2f\n", entry.first, entry.second);
  //return ret;
} // G4DataInterpolation::CreateLookupTable()

// -------------------------------------------------------------------------------------

bool G4DataInterpolation::GetLookupTableEntry(//const std::vector<std::pair<double, double>> &table, 
					      double argument, double *entry) const
{
  //#if 1//_TODAY_
  // Get the tabulated table reference; perform sanity checks;
  //const std::vector<std::pair<double, double>> &qe = u_quantumEfficiency.value();
  unsigned dim = m_LookupTable.size(); if (dim < 2) return false;

  // Find a proper bin; no tricks, they are all equidistant;
  auto const &from = m_LookupTable[0];
  auto const &to = m_LookupTable[dim-1];
  // FIXME: calculate once;
  double emin = from.first;
  double emax = to.first;
  double step = (emax - emin) / (dim - 1);
  int ibin = (int)floor((argument - emin) / step);

  //printf("@Q@ %f vs %f, %f -> %d\n", argument, from.first, to.first, ibin);
  
  // Out of range check;
  if (ibin < 0 || ibin >= int(dim)) return false;

  *entry = m_LookupTable[ibin].second;
  //#endif
  
  return true;
} // G4DataInterpolation::GetLookupTableEntry()

// -------------------------------------------------------------------------------------
#if _TODAY_
bool G4DataInterpolation::QE_pass(double ev, double rand) const
{
#if 1//_TODAY_
  double value = 0.0;
  if (!GetFinelyBinnedTableEntry(m_QE_lookup_table, ev, &value)) return false;

  // Get tabulated QE value, compare against the provided random variable;
  return (rand <= value);//m_QE_lookup_table[ibin].second);
#endif
} // G4DataInterpolation::QE_pass()
#endif
// -------------------------------------------------------------------------------------
#if 0
static double GetQE(const CherenkovPhotonDetector *pd, double eph) 
{  
  return pd->CheckQERange(eph) ? pd->GetQE()->CubicSplineInterpolation(eph) : 0.0;
} // CherenkovSteppingAction::GetQE()
#endif
// -------------------------------------------------------------------------------------

namespace eicrecon {
  IrtDebugging::~IrtDebugging()
  {
    {
      std::lock_guard<std::mutex> lock(m_OutputTreeMutex);
      
      printf("@@@ IrtDebugging::~IrtDebugging() ... %2d\n", m_InstanceCounter);
      m_InstanceCounter--;
#if 1
      if (!m_InstanceCounter) {
	m_OutputFile->cd();
	m_EventTree->Write();

	m_Debug->Write();
	
	m_OutputFile->Close();
      } //if
#endif
    }
  }
  
void IrtDebugging::init(
			CherenkovDetectorCollection*     irt_det_coll//,
			//std::shared_ptr<spdlog::logger>& logger
			)
{

  m_random.SetSeed(0x12345678);//m_cfg.seed);
  m_rngUni = [&](){
    return m_random.Uniform(0., 1.0);
  };
  
  {
    std::lock_guard<std::mutex> lock(m_OutputTreeMutex);
    
    printf("@@@ IrtDebugging::init() ... %2d\n", m_InstanceCounter);
#if 1
    m_Event = new CherenkovEvent();
    m_EventPtr = &m_Event;
      
    if (!m_InstanceCounter) {//m_Event) {
      m_OutputFile = new TFile("qrich-events.root", "RECREATE");
      
      m_EventTree = new TTree("t", "My tree");
      m_EventBranch = m_EventTree->Branch("e", "CherenkovEvent", 0/*&m_Event*/, 16000, 2);
      
      m_Debug = new TH1D("debug", "", 1000, 0, 1000);
    } //if
#endif
    m_Instance = m_InstanceCounter++;
    //m_InstanceCounter++;
  }
  
  {
    auto fcfg = new TFile("qrich-optics.root");//cfname ? cfname : dfname);
    printf("@@@ %p\n", fcfg);
    //if (!fcfg) return;
    //auto geometry = dynamic_cast<CherenkovDetectorCollection*>(fcfg->Get("CherenkovDetectorCollection"));
     m_irt_det_coll = dynamic_cast<CherenkovDetectorCollection*>(fcfg->Get("CherenkovDetectorCollection"));
    printf("@@@ %p\n", m_irt_det_coll);
    //auto fdata = new TFile(dfname);
    //m_Tree = dynamic_cast<TTree*>(fdata->Get("t")); 
    //m_Tree->SetBranchAddress("e", &m_Event);

    //m_RICH = geometry->GetDetector(dname);
  }
  // members
  //+  m_irt_det_coll = irt_det_coll;
#if _TODAY_
  m_log          = logger;

  // print the configuration parameters
  m_cfg.Print(m_log, spdlog::level::debug);

  // inform the user if a cheat mode is enabled
  m_cfg.PrintCheats(m_log);
#endif

  // extract the the relevant `CherenkovDetector`, set to `m_irt_det`
  auto& detectors = m_irt_det_coll->GetDetectors();
  printf("@@@ %ld\n", detectors.size());
  if(detectors.size() == 0)
    throw std::runtime_error("No CherenkovDetectors found in input collection `irt_det_coll`");
  //if(detectors.size() > 1)
  //m_log->warn("IrtCherenkovParticleID currently only supports 1 CherenkovDetector at a time; taking the first");
  auto this_detector = *detectors.begin();
  m_det_name         = this_detector.first;
  m_irt_det          = this_detector.second;
  printf("@@@ %ld radiators defined\n", m_irt_det->Radiators().size());
#if _TODAY_
  m_log->debug("Initializing IrtCherenkovParticleID algorithm for CherenkovDetector '{}'", m_det_name);
#endif

  {
    // FIXME: assume a single photo detector type;
    auto pd = m_irt_det->m_PhotonDetectors[0];

#if 1//_THINK_
    {                      
      const unsigned qeEntries = 26;
      
      // Create HRPPD QE table; use LAPPD #126 from Alexey's March 2022 LAPPD Workshop presentation;
      double WL[qeEntries] = { 160,  180,  200,  220,  240,  260,  280,  300,  320,  340,  360,  380,  400,  
			       420,  440,  460,  480,  500,  520,  540,  560,  580,  600,  620,  640,  660};
      double QE[qeEntries] = {0.25, 0.26, 0.27, 0.30, 0.32, 0.35, 0.36, 0.36, 0.36, 0.36, 0.37, 0.35, 0.30, 
			      0.27, 0.24, 0.20, 0.18, 0.15, 0.13, 0.11, 0.10, 0.09, 0.08, 0.07, 0.05, 0.05};  
      
      double qemax = 0.0, qePhotonEnergy[qeEntries], qeData[qeEntries];
      for(int iq=0; iq<qeEntries; iq++) {
	qePhotonEnergy[iq] = /*dd4hep::eV **/ _MAGIC_CFF_ / (WL[qeEntries - iq - 1] + 0.0);
	qeData        [iq] =                             QE[qeEntries - iq - 1] * _FAKE_QE_DOWNSCALING_FACTOR_;
	
	if (qeData[iq] > qemax) qemax = qeData[iq];
      } //for iq
      
      //pd->SetQE(dd4hep::eV * _MAGIC_CFF_ / WL[qeEntries-1], dd4hep::eV * _MAGIC_CFF_ / WL[0],
      pd->SetQE(_MAGIC_CFF_ / WL[qeEntries-1], _MAGIC_CFF_ / WL[0], 
		// NB: last argument: want a built-in selection of unused photons, which follow the QE(lambda);
		// see CherenkovSteppingAction::UserSteppingAction() for a usage case;
		new G4DataInterpolation(qePhotonEnergy, qeData, qeEntries, 0.0, 0.0), qemax ? 1.0/qemax : 1.0);
      // FIXME: 100 hardcoded;
      pd->GetQE()->CreateLookupTable(100);
    }
#endif
  }
  
  // readout decoding
  //m_cell_mask = m_irt_det->GetReadoutCellMask();
  //m_log->debug("readout cellMask = {:#X}", m_cell_mask);

#if _OLD_
  // rebin refractive index tables to have `m_cfg.numRIndexBins` bins
  m_log->trace("Rebinning refractive index tables to have {} bins",m_cfg.numRIndexBins);
  for(auto [rad_name,irt_rad] : m_irt_det->Radiators()) {
    auto ri_lookup_table_orig = irt_rad->m_ri_lookup_table;
    irt_rad->m_ri_lookup_table.clear();
    irt_rad->m_ri_lookup_table = Tools::ApplyFineBinning( ri_lookup_table_orig, m_cfg.numRIndexBins );
    // m_log->trace("- {}", rad_name);
    // for(auto [energy,rindex] : irt_rad->m_ri_lookup_table) m_log->trace("  {:>5} eV   {:<}", energy, rindex);
  }
#endif

  // build `m_pid_radiators`, the list of radiators to use for PID
  //m_log->debug("Obtain List of Radiators:");
  for(auto [rad_name,irt_rad] : m_irt_det->Radiators()) {
    if(rad_name!="Filter") {
      m_pid_radiators.insert({ std::string(rad_name), irt_rad });
      //m_log->debug("- {}", rad_name.Data());
      
      printf("@@@ %s\n", rad_name.Data());
      radiators[irt_rad] = rad_name.Data();
    }
  }

#if _TODAY_
  // check radiators' configuration, and pass it to `m_irt_det`'s radiators
  for(auto [rad_name,irt_rad] : m_pid_radiators) {
    // find `cfg_rad`, the associated `IrtCherenkovParticleIDConfig` radiator
    auto cfg_rad_it = m_cfg.radiators.find(rad_name);
    if(cfg_rad_it != m_cfg.radiators.end()) {
      auto cfg_rad = cfg_rad_it->second;
      // pass `cfg_rad` params to `irt_rad`, the IRT radiator
      irt_rad->m_ID = Tools::GetRadiatorID(std::string(rad_name));
      irt_rad->m_AverageRefractiveIndex = cfg_rad.referenceRIndex;
      irt_rad->SetReferenceRefractiveIndex(cfg_rad.referenceRIndex);
      if(cfg_rad.attenuation>0)
        irt_rad->SetReferenceAttenuationLength(cfg_rad.attenuation);
      if(cfg_rad.smearing>0) {
        if(cfg_rad.smearingMode=="uniform")
          irt_rad->SetUniformSmearing(cfg_rad.smearing);
        else if(cfg_rad.smearingMode=="gaussian")
          irt_rad->SetGaussianSmearing(cfg_rad.smearing);
        else
          m_log->error("Unknown smearing mode '{}' for {} radiator", cfg_rad.smearingMode, rad_name);
      }
    }
    else
      m_log->error("Cannot find radiator '{}' in IrtCherenkovParticleIDConfig instance", rad_name);
  }

  // get PDG info for the particles we want to identify in PID
  m_log->debug("List of particles for PID:");
  for(auto pdg : m_cfg.pdgList) {
    auto mass = m_particleSvc.particle(pdg).mass;
    m_pdg_mass.insert({ pdg, mass });
    m_log->debug("  {:>8}  M={} GeV", pdg, mass);
  }
#endif
}

void IrtDebugging::process(
			   const IrtDebugging::Input& input,
			   const IrtDebugging::Output& output
			   ) const
{
  printf("@@@ IrtDebugging::process() ...\n");

  // Reset output event structure;
  m_Event->Reset();

  // Intermediate variables, for less typing;
  const auto [in_mc_particles,
	      in_reco_particles,
	      in_mc_reco_associations,
	      in_aerogel_tracks, in_sim_hits] = input;
  auto [out_irt_debug_info] = output;

  // First build MC->reco lookup table;
  std::map<unsigned, std::vector<unsigned>> MCParticle_to_Tracks_lut;
  for(const auto &assoc: *in_mc_reco_associations) {
    // MC particle index in its respective in_mc_particles array;
    unsigned mcid = assoc.getSimID();//.id().index;
    // Reco particle index in its respective in_reco_particles array;
    //auto& rcparticle = assoc.getRec();
    unsigned rcid = assoc.getRecID();//.id().index;
    
    auto rcparticle = (*in_reco_particles)[rcid];//assoc.getRecID()];
    //auto tnum = rcparticle.getTracks().size();
    //printf("tnum: %ld\n", tnum);
    for(auto &track: rcparticle.getTracks())
      MCParticle_to_Tracks_lut[mcid].push_back(track.id().index);
  } //for assoc

  // Then track -> aerogel projection lookup table; FIXME: other radiators; 
  std::map<unsigned, edm4eic::TrackSegment> Track_to_TrackSegment_lut;
  for(auto segment: *in_aerogel_tracks) {
    //auto particle      = particles->at(0);//i_charged_particle);
    //printf("(3)   --> %ld\n", particle.points_size());
    auto track = segment.getTrack();
    Track_to_TrackSegment_lut[track.id().index] = segment;
  } //for particle
    
  // Help optical photons to find their parents;
  std::map<unsigned, ChargedParticle*> MCParticle_to_ChargedParticle;
  
  unsigned counter = 0;
    
  // Create event structure a la standalone pfRICH/IRT code; use MC particles (in_mc_particles)
  // in this first iteration (and only select primary ones); later on should do it probably
  // the same way as in ATHENA IRT codes, where one had an option to build this event
  // structure using reconstructed tracks (in_reco_particles);
  {
    for(const auto &mcparticle: *in_mc_particles) {
      // Deal only with charged primary ones, for the time being; FIXME: low momentum cutoff?;
      if (mcparticle.isCreatedInSimulation() || !mcparticle.getCharge()) continue;
      //printf("charge: %f\n", mcparticle.getCharge());

      unsigned mcid = mcparticle.id().index;
      
      // Now check that MC->reco association exists; for now ignore cases where more than one track
      // is associated with a given MC particle;
      if (MCParticle_to_Tracks_lut.find(mcid) == MCParticle_to_Tracks_lut.end()) continue;
      auto &rctracks = MCParticle_to_Tracks_lut[mcid];
      if (rctracks.size() > 1) continue;
      auto &rctrack = rctracks[0];

      // Now add a charged particle to the event structure; 'true': primary;
      auto particle = new ChargedParticle(mcparticle.getPDG(), true);
      // FIXME: check units;
      particle->SetVertexPosition(/*(1/dd4hep::mm)  **/ Tools::PodioVector3_to_TVector3(mcparticle.getVertex()));
      particle->SetVertexMomentum(/*(1/dd4hep::GeV) **/ Tools::PodioVector3_to_TVector3(mcparticle.getMomentum()));
      particle->SetVertexTime    (/*(1/dd4hep::ns)  **/ mcparticle.getTime());
      
      m_Event->AddChargedParticle(particle);
      MCParticle_to_ChargedParticle[mcid] = particle;

      {
	auto aerogel = m_irt_det->GetRadiator("Aerogel");
	
	auto history = new RadiatorHistory();
	particle->StartRadiatorHistory(std::make_pair(aerogel, history));

	auto segment = Track_to_TrackSegment_lut[rctrack];//.id().index];
	for(const auto& point : segment.getPoints()) {
	  TVector3 position = Tools::PodioVector3_to_TVector3(point.position);
	  TVector3 momentum = Tools::PodioVector3_to_TVector3(point.momentum);
	  
	  //irt_rad->AddLocation(position, momentum);
	  //Tools::PrintTVector3(m_log, " point: x", position);
	  //Tools::PrintTVector3(m_log, "        p", momentum);
#if 1
	  printf("x=%7.2f y=%7.2f z=%7.2f -> px=%8.4f py=%8.4f pz=%7.2f\n",
		 position.X(), position.Y(), position.Z(), 
		 momentum.X(), momentum.Y(), momentum.Z());
#endif

	  auto step = new ChargedParticleStep(position, momentum);
	  history->AddStep(step);
	} //for point
      }
    } //for mcparticle

#if 0
#if 0
    TVector3 momentum = Tools::PodioVector3_to_TVector3(track.getMomentum());
    printf("%3d / %3d (%3d) vs %3d\n", track.id().collectionID, particle.id().collectionID, track.id().index, in_reco_particles->getID());//momentum.Mag());
    for(const auto& point : particle.getPoints()) {
      TVector3 position = Tools::PodioVector3_to_TVector3(point.position);
      TVector3 momentum = Tools::PodioVector3_to_TVector3(point.momentum);
      
      //irt_rad->AddLocation(position, momentum);
      //Tools::PrintTVector3(m_log, " point: x", position);
      //Tools::PrintTVector3(m_log, "        p", momentum);
#if 1
      printf("x=%7.2f y=%7.2f z=%7.2f -> px=%8.4f py=%8.4f pz=%7.2f\n",
	     position.X(), position.Y(), position.Z(), 
	     momentum.X(), momentum.Y(), momentum.Z());
#endif
    } //for point
#endif
#endif
    
    // Now loop through simulated hits;
    for(auto mchit: *in_sim_hits) {
      //auto cell_id   = mchit.getCellID();
      //uint64_t sensor_id = cell_id & m_irt_det->m_cell_mask;
      //printf("cell: %lX\n", cell_id);

      printf("dE: %7.2f\n", 1E9 * mchit.getEDep());
      
      // Get photon which created this hit; filter out charged particles;
      auto &mcparticle = mchit.getMCParticle();
      //printf("%4d -> %d\n", mcparticle.id().index, mcparticle.getPDG());
      if (mcparticle.getPDG() != -22) continue;

      counter++;
      
      // Create an optical photon class instance and populate it; units: [mm], [ns], [eV];
      auto photon = new OpticalPhoton();

      // Information provided by the hit itself: detection position and time;
      photon->SetDetectionPosition(/*(1/dd4hep::mm)  **/ Tools::PodioVector3_to_TVector3(mchit.getPosition()));
      photon->SetDetectionTime    (/*(1/dd4hep::ns)  **/ mchit.getTime());

      // Information inherited from photon MCParticle; FIXME: units?;
      photon->SetVertexPosition   (/*(1/dd4hep::mm)  **/ Tools::PodioVector3_to_TVector3(mcparticle.getVertex()));
      photon->SetVertexMomentum   (/*(1/dd4hep::GeV) **/ 1E9 * Tools::PodioVector3_to_TVector3(mcparticle.getMomentum()));
      photon->SetVertexTime       (/*(1/dd4hep::ns)  **/ mcparticle.getTime());
      //printf("time: %f %f\n",  photon->GetVertexTime(), photon->GetDetectionTime());
      //auto &vtx1 = photon->GetVertexPosition(), &vtx2 = photon->GetDetectionPosition();
      //printf("%f %f %f-> %f %f %f\n", vtx1[0], vtx1[1], vtx1[2], vtx2[0], vtx2[1], vtx2[2]);
      //printf("p: %f\n", photon->GetVertexMomentum().Mag());
      
      // photon->SetVertexParentMomentum((1/GeV)*TVector3(pparent.x(), pparent.y(), pparent.z()));

      // FIXME: (0,0,1) or (0,0,-1)?; should be different for e-endcap?;
      TVector3 vtx = /*(1/dd4hep::mm)**/Tools::PodioVector3_to_TVector3(mcparticle.getVertex()), n0(0,0,1);
      //printf("%p -> %d\n", m_irt_det, m_irt_det->GetSector(vtx));
      auto radiator = m_irt_det->GuessRadiator(vtx, n0); // assume IP is at (0,0,0)
      //printf("%s\n", radiator ? radiators[radiator].c_str() : "");
      //printf("%s\n", rptr.first.Data());

      auto parents = mcparticle.getParents();
      if (parents.size() != 1) continue;
      unsigned parent_id = mcparticle.parents_begin()->id().index;
      if (MCParticle_to_ChargedParticle.find(parent_id) == MCParticle_to_ChargedParticle.end()) continue;
      auto parent = MCParticle_to_ChargedParticle[parent_id];
	
      // FIXME: a hack for now;
      //if (!radiator || strcmp(radiators[radiator].c_str(), "Aerogel")) continue;
      if (radiator) {
	printf("%s -> p: %f\n", radiators[radiator].c_str(), photon->GetVertexMomentum().Mag());
	//photon->SetVertexAttenuationLength(GetAttenuationLength(radiator, e));
	photon->SetVertexAttenuationLength(40.0);
	//photon->SetVertexRefractiveIndex (GetRefractiveIndex(radiator, e));
	photon->SetVertexRefractiveIndex (1.040);
	
	auto history = parent->FindRadiatorHistory(radiator);
	// FIXME: this happens with the sensor window volumes; why?;
	if (!history) {
	  history = new RadiatorHistory();
	  parent->StartRadiatorHistory(std::make_pair(radiator, history));
	} //if
	
	history->AddOpticalPhoton(photon);
      } else {
	if (m_irt_det_coll->CheckBits(_STORE_ORPHAN_PHOTONS_))
	  parent->AddOrphanPhoton(photon);
      } //if

      // FIXME: this is a hack;
      //printf("--> %ld photon detector(s)\n", m_irt_det->m_PhotonDetectors.size());
      if (m_irt_det->m_PhotonDetectors.size() != 1) continue;
      //auto pd = m_Geometry->FindPhotonDetector(lto);
      auto pd = m_irt_det->m_PhotonDetectors[0];
      
      if (pd) {
	photon->SetPhotonDetector(pd);
	// FIXME: a hack;
	//photon->SetVolumeCopy(xto->GetTouchable()->GetCopyNumber(pd->GetCopyIdentifierLevel()));
	photon->SetVolumeCopy(0);

	//printf("-> %f %f\n", pd->GetGeometricEfficiency(), pd->GetScaleFactor());
#if 0//1
	double rndm = m_rngUni();
	(rndm > 0.90) ? photon->SetDetected(true) :  photon->SetCalibrationFlag();
	printf("@@@ %7.5f -> %d %d\n", rndm, photon->WasDetected(), photon->IsUsefulForCalibration());
#else
	// The logic behind this multiplication and division by the same number is 
	// to select calibration photons, which originate from the same QE(lambda) 
	// parent distribution, but do not pass the overall efficiency test;
	{
	  //auto *QE = pd->GetQE();
	  //double QE = 0.20, scale_factor = 1/QE;
	  double e = photon->GetVertexMomentum().Mag(), qe;
	  if (!pd->GetQE()->GetLookupTableEntry(e, &qe)) qe = 0.0;
	  printf("@Q@ %7.2f -> %7.2f\n", e, qe);
	  
	  if (qe*pd->GetScaleFactor() > m_rngUni()) {
	  //if (QE*scale_factor > m_rngUni()) {
	    if (pd->GetGeometricEfficiency()/pd->GetScaleFactor() > m_rngUni())
	      //if (pd->GetGeometricEfficiency()/scale_factor > m_rngUni())
	      photon->SetDetected(true);
	    else
	      photon->SetCalibrationFlag();
	  } //if
	}
#endif
      } //if


      //? if (!info->Parent()) m_EventPtr->AddOrphanPhoton(photon);
    } //for hit
  }



  


  
#if _OLD_
#if _TODAY_
  // logging
  m_log->trace("{:=^70}"," call IrtCherenkovParticleID::AlgorithmProcess ");
  m_log->trace("number of raw sensor hits: {}", in_raw_hits->size());
  m_log->trace("number of raw sensor hit with associated photons: {}", in_hit_assocs->size());
#endif

  std::map<std::string, const edm4eic::TrackSegmentCollection*> in_charged_particles{
    {"Aerogel", in_aerogel_tracks}//,
    //{"Gas", in_gas_tracks},
    //{"Merged", in_merged_tracks},
  };

  // start output collections
  std::map<std::string, edm4eic::IrtDebugInfoCollection*> out_cherenkov_pids{
    //{"Aerogel", out_aerogel_particleIDs}//,
    {"Aerogel", out_irt_debug_info}//,
    //{"Gas", out_gas_particleIDs}
  };

  printf("(1) --> %ld\n", in_charged_particles.size());
  // FIXME: deal with just aerogel for now;
  const auto &particles = in_charged_particles.begin()->second;
  printf("(2) --> %ld\n", particles->size());
  for(auto particle: *particles) {
    //auto particle      = particles->at(0);//i_charged_particle);
    printf("(3)   --> %ld\n", particle.points_size());
    auto track = particle.getTrack();
    TVector3 momentum = Tools::PodioVector3_to_TVector3(track.getMomentum());
    printf("%3d / %3d (%3d) vs %3d\n", track.id().collectionID, particle.id().collectionID, track.id().index, in_reco_particles->getID());//momentum.Mag());
    for(const auto& point : particle.getPoints()) {
      TVector3 position = Tools::PodioVector3_to_TVector3(point.position);
      TVector3 momentum = Tools::PodioVector3_to_TVector3(point.momentum);
      
      //irt_rad->AddLocation(position, momentum);
      //Tools::PrintTVector3(m_log, " point: x", position);
      //Tools::PrintTVector3(m_log, "        p", momentum);
#if 1
      printf("x=%7.2f y=%7.2f z=%7.2f -> px=%8.4f py=%8.4f pz=%7.2f\n",
	     position.X(), position.Y(), position.Z(), 
	     momentum.X(), momentum.Y(), momentum.Z());
#endif
    } //for point
  } //for particle
  
#if 0//_TODAY_
  printf("(4) --> %ld\n", in_sim_hits->size());
  for(auto hit: *in_sim_hits) {
    TVector3 position = Tools::PodioVector3_to_TVector3(hit.getPosition());
    //printf("x=%7.2f y=%7.2f z=%7.2f\n", position.X(), position.Y(), position.Z());
  } //for hit
#endif
  
  auto info = out_irt_debug_info->create();
  info.setDummy(777. + m_Instance*10);


  
  // check `in_charged_particles`: each radiator should have the same number of TrackSegments
  std::unordered_map<std::size_t, std::size_t> in_charged_particle_size_distribution;
  for(const auto& [rad_name, in_charged_particle] : in_charged_particles) {
    ++in_charged_particle_size_distribution[in_charged_particle->size()];
  }
#if _ORIG_
  if (in_charged_particle_size_distribution.size() != 1) {
    std::vector<size_t> in_charged_particle_sizes;
    std::transform(in_charged_particles.begin(), in_charged_particles.end(),
      std::back_inserter(in_charged_particle_sizes),
      [](const auto& in_charged_particle) { return in_charged_particle.second->size(); });
    m_log->error("radiators have differing numbers of TrackSegments {}", fmt::join(in_charged_particle_sizes, ", "));
    return;
  }
#endif

#if 0
  {
    for(const auto& assoc : *in_mc_particles) {
      //for(const auto& assoc : in_mc_particles) {
      //m_log->trace("{:-^50}"," Particle ");

      auto simpart_p = edm4hep::utils::p(assoc);//.getMomentum());//simpart);
      printf("p (%d): %f\n", assoc.isCreatedInSimulation(), simpart_p);//, recpart_p, recpart_p - simpart_p);
      info.setP(simpart_p);//assoc.getMomentum().x());//100.*(recpart_p - simpart_p)/simpart_p);
      //auto simpart_p = edm4hep::utils::p(assoc);//simpart);
      
    } //for assoc
  }
#endif
#if 0
  {
    for(const auto& assoc : *in_reco_particles) {
      //for(const auto& assoc : in_mc_particles) {
      //m_log->trace("{:-^50}"," Particle ");

      auto simpart_p = edm4hep::utils::p(assoc);//.getMomentum());//simpart);
      printf("p: %f\n", simpart_p);//, recpart_p, recpart_p - simpart_p);
      info.setP(simpart_p);//assoc.getMomentum().x());//100.*(recpart_p - simpart_p)/simpart_p);
      //auto simpart_p = edm4hep::utils::p(assoc);//simpart);
      
    } //for assoc
  }
#endif
  
  // Loop over charged particles;
#if 0//_NEW_
  {
    for(const auto& assoc : *in_mc_reco_associations) {
      //for(const auto& assoc : in_mc_particles) {
      //m_log->trace("{:-^50}"," Particle ");
      auto& simpart = assoc.getSim();
      auto& recpart = assoc.getRec();//.getTracks.at(0);
      //printf("@@@ %3d %3d\n", simpart.isAvailable(), recpart.isAvailable());
      printf("@@@ %3d %3d\n", assoc.getSimID(), assoc.getRecID());//simpart.isAvailable(), recpart.isAvailable());

      auto track = (*in_reco_particles)[assoc.getRecID()];
      
      //auto simpart_p = edm4hep::utils::p(simpart);
      //printf("%f -> %d\n", simpart_p, recpart.getPdg());

      //auto tnum = recpart.getTracks().size();
      //printf("tnum: %ld\n", tnum);
      //auto &track = *recpart.tracks_begin();
      
#if 1
      auto simpart_p = edm4hep::utils::p(simpart);
      auto recpart_p = edm4hep::utils::p(track);//recpart); // unused
      printf("%f %f -> %f\n", simpart_p, recpart_p, recpart_p - simpart_p);
      info.setPdiff(100.*(recpart_p - simpart_p)/simpart_p);
#endif
    } //for assoc
  }
#endif

#if _OLD_
  // loop over charged particles ********************************************
  //m_log->trace("{:#<70}","### CHARGED PARTICLES ");
  std::size_t num_charged_particles = in_charged_particle_size_distribution.begin()->first;
  printf("(5) --> Here! %2ld\n", num_charged_particles);
  for(long i_charged_particle=0; i_charged_particle<num_charged_particles; i_charged_particle++) {
    //m_log->trace("{:-<70}", fmt::format("--- charged particle #{} ", i_charged_particle));

    // start an `irt_particle`, for `IRT`
    auto irt_particle = std::make_unique<ChargedParticle>();

    // loop over radiators
    for(auto [rad_name,irt_rad] : m_pid_radiators) {

      printf("(6) --> Here! %s\n", rad_name.c_str());//num_charged_particles);
#if _TODAY_
      // get the `charged_particle` for this radiator
      auto charged_particle_list_it = in_charged_particles.find(rad_name);
      if(charged_particle_list_it == in_charged_particles.end()) {
        //m_log->error("Cannot find radiator '{}' in `in_charged_particles`", rad_name);
        continue;
      }
      const auto *charged_particle_list = charged_particle_list_it->second;
      auto charged_particle      = charged_particle_list->at(i_charged_particle);

      // set number of bins for this radiator and charged particle
      if(charged_particle.points_size()==0) {
        //m_log->trace("No propagated track points in radiator '{}'", rad_name);
        continue;
      }

      
      printf("(7) --> Here! %2ld\n", i_charged_particle);
#if _TODAY_
      irt_rad->SetTrajectoryBinCount(charged_particle.points_size() - 1);

      // start a new IRT `RadiatorHistory`
      // - must be a raw pointer for `irt` compatibility
      // - it will be destroyed when `irt_particle` is destroyed
      auto *irt_rad_history = new RadiatorHistory();
      irt_particle->StartRadiatorHistory({ irt_rad, irt_rad_history });

      // loop over `TrackPoint`s of this `charged_particle`, adding each to the IRT radiator
      irt_rad->ResetLocations();
      m_log->trace("TrackPoints in '{}' radiator:", rad_name);
      for(const auto& point : charged_particle.getPoints()) {
        TVector3 position = Tools::PodioVector3_to_TVector3(point.position);
        TVector3 momentum = Tools::PodioVector3_to_TVector3(point.momentum);
        irt_rad->AddLocation(position, momentum);
        Tools::PrintTVector3(m_log, " point: x", position);
        Tools::PrintTVector3(m_log, "        p", momentum);
      }


      // loop over raw hits ***************************************************
      m_log->trace("{:#<70}","### SENSOR HITS ");
      for(const auto& raw_hit : *in_raw_hits) {

        // get MC photon(s), typically only used by cheat modes or trace logging
        // - loop over `in_hit_assocs`, searching for the matching hit association
        // - will not exist for noise hits
        edm4hep::MCParticle mc_photon;
        bool mc_photon_found = false;
        if(m_cfg.cheatPhotonVertex || m_cfg.cheatTrueRadiator) {
          for(const auto& hit_assoc : *in_hit_assocs) {
            if(hit_assoc.getRawHit().isAvailable()) {
              if(hit_assoc.getRawHit().id() == raw_hit.id()) {
#if EDM4EIC_VERSION_MAJOR >= 6
                mc_photon = hit_assoc.getSimHit().getMCParticle();
#else
                // hit association found, get the MC photon and break the loop
                if(hit_assoc.simHits_size() > 0) {
                  mc_photon = hit_assoc.getSimHits(0).getMCParticle();
#endif
                  mc_photon_found = true;
                  if(mc_photon.getPDG() != -22)
                    m_log->warn("non-opticalphoton hit: PDG = {}",mc_photon.getPDG());
#if EDM4EIC_VERSION_MAJOR >= 6
#else
                }
                else if(m_cfg.CheatModeEnabled())
                  m_log->error("cheat mode enabled, but no MC photons provided");
#endif
                break;
              }
            }
          }
        }

        // cheat mode, for testing only: use MC photon to get the actual radiator
        if(m_cfg.cheatTrueRadiator && mc_photon_found) {
          auto vtx    = Tools::PodioVector3_to_TVector3(mc_photon.getVertex());
          auto mc_rad = m_irt_det->GuessRadiator(vtx, vtx); // assume IP is at (0,0,0)
          if(mc_rad != irt_rad) continue; // skip this photon, if not from radiator `irt_rad`
          Tools::PrintTVector3(m_log, fmt::format("cheat: radiator '{}' determined from photon vertex", rad_name), vtx);
        }

        // get sensor and pixel info
        // FIXME: signal and timing cuts (ADC, TDC, ToT, ...)
        auto     cell_id   = raw_hit.getCellID();
        uint64_t sensor_id = cell_id & m_cell_mask;
        TVector3 pixel_pos = m_irt_det->m_ReadoutIDToPosition(cell_id);

        // trace logging
        if(m_log->level() <= spdlog::level::trace) {
          m_log->trace("cell_id={:#X}  sensor_id={:#X}", cell_id, sensor_id);
          Tools::PrintTVector3(m_log, "pixel position", pixel_pos);
          if(mc_photon_found) {
            TVector3 mc_endpoint = Tools::PodioVector3_to_TVector3(mc_photon.getEndpoint());
            Tools::PrintTVector3(m_log, "photon endpoint", mc_endpoint);
            m_log->trace("{:>30} = {}", "dist( pixel,  photon )", (pixel_pos  - mc_endpoint).Mag());
          }
          else m_log->trace("  no MC photon found; probably a noise hit");
        }

        // start new IRT photon
        auto *irt_sensor = m_irt_det->m_PhotonDetectors[0]; // NOTE: assumes one sensor type
        auto *irt_photon = new OpticalPhoton(); // new raw pointer; it will also be destroyed when `irt_particle` is destroyed
        irt_photon->SetVolumeCopy(sensor_id);
        irt_photon->SetDetectionPosition(pixel_pos);
        irt_photon->SetPhotonDetector(irt_sensor);
        irt_photon->SetDetected(true);

        // cheat mode: get photon vertex info from MC truth
        if((m_cfg.cheatPhotonVertex || m_cfg.cheatTrueRadiator) && mc_photon_found) {
          irt_photon->SetVertexPosition(Tools::PodioVector3_to_TVector3(mc_photon.getVertex()));
          irt_photon->SetVertexMomentum(Tools::PodioVector3_to_TVector3(mc_photon.getMomentum()));
        }

        // cheat mode: retrieve a refractive index estimate; it is not exactly the one, which
        // was used in GEANT, but should be very close
        if(m_cfg.cheatPhotonVertex) {
          double ri;
          auto mom    = 1e9 * irt_photon->GetVertexMomentum().Mag();
          auto ri_set = Tools::GetFinelyBinnedTableEntry(irt_rad->m_ri_lookup_table, mom, &ri);
          if(ri_set) {
            irt_photon->SetVertexRefractiveIndex(ri);
            m_log->trace("{:>30} = {}", "refractive index", ri);
          }
          else
            m_log->warn("Tools::GetFinelyBinnedTableEntry failed to lookup refractive index for momentum {} eV", mom);
        }

        // add each `irt_photon` to the radiator history
        // - unless cheating, we don't know which photon goes with which
        // radiator, thus we add them all to each radiator; the radiators'
        // photons are mixed in `ChargedParticle::PIDReconstruction`
        irt_rad_history->AddOpticalPhoton(irt_photon);
        /* FIXME: this considers ALL of the `irt_photon`s... we can limit this
         * once we add the ability to get a fiducial volume for each track, i.e.,
         * a region of sensors where we expect to see this `irt_particle`'s
         * Cherenkov photons; this should also combat sensor noise
         */
      } // end `in_hit_assocs` loop

#endif
#endif
    } // end radiator loop

#if _TODAY_
    // particle identification +++++++++++++++++++++++++++++++++++++++++++++++++++++

    // define a mass hypothesis for each particle we want to check
    m_log->trace("{:+^70}"," PARTICLE IDENTIFICATION ");
    CherenkovPID irt_pid;
    std::unordered_map<int,MassHypothesis*> pdg_to_hyp; // `pdg` -> hypothesis
    for(auto [pdg,mass] : m_pdg_mass) {
      irt_pid.AddMassHypothesis(mass);
      pdg_to_hyp.insert({ pdg, irt_pid.GetHypothesis(irt_pid.GetHypothesesCount()-1) });
    }

    // run IRT PID
    irt_particle->PIDReconstruction(irt_pid);
    m_log->trace("{:-^70}"," IRT RESULTS ");

    // loop over radiators
    for(auto [rad_name,irt_rad] : m_pid_radiators) {
      m_log->trace("-> {} Radiator (ID={}):", rad_name, irt_rad->m_ID);

      // Cherenkov angle (theta) estimate
      unsigned npe        = 0;
      double   rindex_ave = 0.0;
      double   energy_ave = 0.0;
      std::vector<std::pair<double,double>> phot_theta_phi;

      // loop over this radiator's photons, and decide which to include in the theta estimate
      auto *irt_rad_history = irt_particle->FindRadiatorHistory(irt_rad);
      if(irt_rad_history==nullptr) {
        m_log->trace("  No radiator history; skip");
        continue;
      }
      m_log->trace("  Photoelectrons:");
      for(auto *irt_photon : irt_rad_history->Photons()) {

        // check whether this photon was selected by at least one mass hypothesis
        bool photon_selected = false;
        for(auto irt_photon_sel : irt_photon->_m_Selected) {
          if(irt_photon_sel.second == irt_rad) {
            photon_selected = true;
            break;
          }
        }
        if(!photon_selected) continue;

        // trace logging
        Tools::PrintTVector3(
            m_log,
            fmt::format("- sensor_id={:#X}: hit",irt_photon->GetVolumeCopy()),
            irt_photon->GetDetectionPosition()
            );
        Tools::PrintTVector3(m_log, "photon vertex", irt_photon->GetVertexPosition());

        // get this photon's theta and phi estimates
        auto phot_theta = irt_photon->_m_PDF[irt_rad].GetAverage();
        auto phot_phi   = irt_photon->m_Phi[irt_rad];

        // add to the total
        npe++;
        phot_theta_phi.emplace_back( phot_theta, phot_phi );
        if(m_cfg.cheatPhotonVertex) {
          rindex_ave += irt_photon->GetVertexRefractiveIndex();
          energy_ave += irt_photon->GetVertexMomentum().Mag();
        }

      } // end loop over this radiator's photons

      // compute averages
      if(npe>0) {
        rindex_ave /= npe;
        energy_ave /= npe;
      }

      // fill photon info
      auto out_cherenkov_pid = out_cherenkov_pids.at(rad_name)->create();
      out_cherenkov_pid.setNpe(static_cast<decltype(edm4eic::CherenkovParticleIDData::npe)>(npe));
      out_cherenkov_pid.setRefractiveIndex(static_cast<decltype(edm4eic::CherenkovParticleIDData::refractiveIndex)>(rindex_ave));
      out_cherenkov_pid.setPhotonEnergy(static_cast<decltype(edm4eic::CherenkovParticleIDData::photonEnergy)>(energy_ave));
      for(auto [phot_theta,phot_phi] : phot_theta_phi)
        out_cherenkov_pid.addToThetaPhiPhotons(edm4hep::Vector2f{
            static_cast<float>(phot_theta),
            static_cast<float>(phot_phi)
            });

      // relate mass hypotheses
      for(auto [pdg,mass] : m_pdg_mass) {

        // get hypothesis results
        auto *irt_hypothesis = pdg_to_hyp.at(pdg);
        auto hyp_weight     = irt_hypothesis->GetWeight(irt_rad);
        auto hyp_npe        = irt_hypothesis->GetNpe(irt_rad);

        // fill `ParticleID` output collection
        edm4eic::CherenkovParticleIDHypothesis out_hypothesis;
        out_hypothesis.PDG    = static_cast<decltype(edm4eic::CherenkovParticleIDHypothesis::PDG)>(pdg);
        out_hypothesis.weight = static_cast<decltype(edm4eic::CherenkovParticleIDHypothesis::weight)>(hyp_weight);
        out_hypothesis.npe    = static_cast<decltype(edm4eic::CherenkovParticleIDHypothesis::npe)>(hyp_npe);

        // relate
        out_cherenkov_pid.addToHypotheses(out_hypothesis);

      } // end hypothesis loop

      // logging
      Tools::PrintCherenkovEstimate(m_log, out_cherenkov_pid);

      // relate charged particle projection
      auto charged_particle_list_it = in_charged_particles.find("Merged");
      if(charged_particle_list_it != in_charged_particles.end()) {
        const auto *charged_particle_list = charged_particle_list_it->second;
        auto charged_particle      = charged_particle_list->at(i_charged_particle);
        out_cherenkov_pid.setChargedParticle(charged_particle);
      }
      else
        m_log->error("Cannot find radiator 'Merged' in `in_charged_particles`");

      // relate hit associations
      for(const auto& hit_assoc : *in_hit_assocs)
        out_cherenkov_pid.addToRawHitAssociations(hit_assoc);

    } // end radiator loop

    /* NOTE: `unique_ptr irt_particle` goes out of scope and will now be destroyed, and along with it:
     * - raw pointer `irt_rad_history` for each radiator
     * - all `irt_photon` raw pointers
     */

#endif

  } // end `in_charged_particles` loop
#endif
#endif
  
#if 1//_NOW_
  {
    std::lock_guard<std::mutex> lock(m_OutputTreeMutex);
    
    m_EventBranch->SetAddress(m_EventPtr);
    m_EventTree->Fill();

    //printf("m_Debug: %p\n", m_Debug
    m_Debug->Fill(counter);
  }
#endif
}

} // namespace eicrecon
#endif
