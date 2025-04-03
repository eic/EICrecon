//
// Copyright 2025, Alexander Kiselev
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#define _ELECTRON_GOING_ENDCAP_CASE_

#ifdef _ELECTRON_GOING_ENDCAP_CASE_
static const double sign = -1.0;
#else
static const double sign =  1.0;
#endif

#include <mutex>

#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TBranch.h>
#include <TVector3.h>

#include <edm4hep/SimTrackerHitCollection.h>
#include "algorithms/pid/Tools.h"

#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4hep/utils/kinematics.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackSegment.h>

#include <IRT/ChargedParticle.h>
#include "IRT/CherenkovEvent.h"

#include "G4DataInterpolation.h"
#include "IrtDebugging.h"

// IrtDebugging::process() is const -> move this mutex out for the time being;
static std::mutex m_OutputTreeMutex;

TFile *eicrecon::IrtDebugging::m_OutputFile = 0;
TTree *eicrecon::IrtDebugging::m_EventTree = 0;
TBranch *eicrecon::IrtDebugging::m_EventBranch = 0;
unsigned eicrecon::IrtDebugging::m_InstanceCounter = 0;
TH1D *eicrecon::IrtDebugging::m_Debug = 0;

// FIXME: move to a different place;
#define _MAGIC_CFF_                              (1239.8)
#define _FAKE_QE_DOWNSCALING_FACTOR_          (30.0/37.0)
#define _REFERENCE_WAVE_LENGTH_                   (390.0)

// -------------------------------------------------------------------------------------

namespace eicrecon {
  IrtDebugging::~IrtDebugging()
  {
    std::lock_guard<std::mutex> lock(m_OutputTreeMutex);
      
    //printf("@@@ IrtDebugging::~IrtDebugging() ... %2d\n", m_InstanceCounter);
    m_InstanceCounter--;
    
    if (!m_InstanceCounter) {
      m_OutputFile->cd();
      m_EventTree->Write();

      // Write an optics configuration copy into the output event tree; this modified version
      // will in particular contain properly assigned m_ReferenceRefractiveIndex values;
      m_irt_det_coll->Write();
      
      m_Debug->Write();
      
      m_OutputFile->Close();
    } //if
  } // IrtDebugging::~IrtDebugging()
  
  // -------------------------------------------------------------------------------------
  
  void IrtDebugging::init(RichGeo_service &service, std::shared_ptr<spdlog::logger>& logger)
  {
    m_random.SetSeed(0x12345678);//m_cfg.seed);
    m_rngUni = [&](){
      return m_random.Uniform(0., 1.0);
    };
  
    {
      std::lock_guard<std::mutex> lock(m_OutputTreeMutex);
    
      printf("@@@ IrtDebugging::init() ... %2d\n", m_InstanceCounter);

      m_Event = new CherenkovEvent();
      m_EventPtr = &m_Event;
      
      if (!m_InstanceCounter) {
	// FIXME: hardcoded;
	m_OutputFile = new TFile("qrich-events.root", "RECREATE");
	//m_OutputFile = new TFile("pfrich-events.root", "RECREATE");
	
	m_EventTree = new TTree("t", "My tree");
	m_EventBranch = m_EventTree->Branch("e", "CherenkovEvent", 0/*&m_Event*/, 16000, 2);
	
	m_Debug = new TH1D("debug", "", 1000, 0, 1000);
      } //if

      m_Instance = m_InstanceCounter++;
    }
  
    {
      // FIXME: hardcoded; FIXME: check existence;
      auto fcfg = new TFile("qrich-optics.root");
      //auto fcfg = new TFile("pfrich-optics.root");
      m_irt_det_coll = dynamic_cast<CherenkovDetectorCollection*>(fcfg->Get("CherenkovDetectorCollection"));
    }
    
    m_log = logger;

    // Extract the the relevant `CherenkovDetector`, set to `m_irt_det`;
    auto& detectors = m_irt_det_coll->GetDetectors();
    //printf("@@@ %ld\n", detectors.size());
    if(detectors.size() == 0)
      throw std::runtime_error("No CherenkovDetectors found in input collection `irt_det_coll`");
    auto this_detector = *detectors.begin();
    m_det_name         = this_detector.first;
    m_irt_det          = this_detector.second;
    //printf("@@@ %ld radiators defined\n", m_irt_det->Radiators().size());
    //m_log->debug("Initializing IrtCherenkovParticleID algorithm for CherenkovDetector '{}'", m_det_name);

    {
      auto *det = service.GetDD4hepGeo();

      for(auto [name,rad] : m_irt_det->Radiators()) {
	printf("@R@ %s\n", name.Data());
	const auto *rindex_matrix = det->material(rad->GetAlternativeMaterialName()).property("RINDEX");
	if (rindex_matrix) {
	  unsigned dim = rindex_matrix->GetRows();
	  double e[dim], ri[dim];
	  for(unsigned row=0; row<rindex_matrix->GetRows(); row++) {
	    e [row] = rindex_matrix->Get(row,0) / dd4hep::eV;
	    ri[row] = rindex_matrix->Get(row,1);
	    
	    //printf("%7.3f %7.3f\n", energy, rindex);
	  } //for row
	  
	  {
	    auto ptr = rad->m_RefractiveIndex = new G4DataInterpolation(e, ri, dim);
	    ptr->CreateLookupTable(100);
	    
	    double e = _MAGIC_CFF_ / _REFERENCE_WAVE_LENGTH_;
	    double value = ptr->GetInterpolatedValue(e, G4DataInterpolation::FirstOrder);	
	    //printf("@W@ -> %d -> %f vs %f\n", ptr->WithinRange(e), value, rad->GetReferenceRefractiveIndex());
	    
	    rad->SetReferenceRefractiveIndex(value);
	  }
	} //if
      } //for radiators
    }
    
    {
      // FIXME: assume a single photo detector type;
      auto pd = m_irt_det->m_PhotonDetectors[0];
      
      {                      
	const unsigned qeEntries = 26;
	
	// Create HRPPD QE table; use LAPPD #126 from Alexey's March 2022 LAPPD Workshop presentation;
	double WL[qeEntries] = { 160,  180,  200,  220,  240,  260,  280,  300,  320,  340,  360,  380,  400,  
				 420,  440,  460,  480,  500,  520,  540,  560,  580,  600,  620,  640,  660};
	double QE[qeEntries] = {0.25, 0.26, 0.27, 0.30, 0.32, 0.35, 0.36, 0.36, 0.36, 0.36, 0.37, 0.35, 0.30, 
				0.27, 0.24, 0.20, 0.18, 0.15, 0.13, 0.11, 0.10, 0.09, 0.08, 0.07, 0.05, 0.05};  
	
	double qemax = 0.0, qePhotonEnergy[qeEntries], qeData[qeEntries];
	for(int iq=0; iq<qeEntries; iq++) {
	  qePhotonEnergy[iq] = _MAGIC_CFF_ / (WL[qeEntries - iq - 1] + 0.0);
	  qeData        [iq] =                QE[qeEntries - iq - 1] * _FAKE_QE_DOWNSCALING_FACTOR_;
	  
	  if (qeData[iq] > qemax) qemax = qeData[iq];
	} //for iq
	
	pd->SetQE(_MAGIC_CFF_ / WL[qeEntries-1], _MAGIC_CFF_ / WL[0], 
		  // NB: last argument: want a built-in selection of unused photons, which follow the QE(lambda);
		  // see CherenkovSteppingAction::UserSteppingAction() for a usage case;
		  new G4DataInterpolation(qePhotonEnergy, qeData, qeEntries/*, 0.0, 0.0*/), qemax ? 1.0/qemax : 1.0);
	// FIXME: 100 hardcoded;
	pd->GetQE()->CreateLookupTable(100);
      }
    }
  } // IrtDebugging::init()

  // -------------------------------------------------------------------------------------
  
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
      unsigned rcid = assoc.getRecID();//.id().index;
    
      auto rcparticle = (*in_reco_particles)[rcid];//assoc.getRecID()];
      //auto tnum = rcparticle.getTracks().size();
      printf("tnum: %ld\n", rcparticle.getTracks().size());
      for(auto &track: rcparticle.getTracks())
	MCParticle_to_Tracks_lut[mcid].push_back(track.id().index);
    } //for assoc

    // Then track -> aerogel projection lookup table; FIXME: other radiators; 
    std::map<unsigned, edm4eic::TrackSegment> Track_to_TrackSegment_lut;
    printf("aerogel track group(s): %ld\n", (*in_aerogel_tracks).size());
    for(auto segment: *in_aerogel_tracks) {
      auto track = segment.getTrack();
      printf("(3)   --> %d\n", track.id().index);
      Track_to_TrackSegment_lut[track.id().index] = segment;
    } //for particle
    
    // Help optical photons to find their parents;
    std::map<unsigned, ChargedParticle*> MCParticle_to_ChargedParticle;
    
    unsigned counter = 0;
    
    // Create event structure a la standalone pfRICH/IRT code; use MC particles (in_mc_particles)
    // in this first iteration (and only select primary ones); later on should do it probably
    // the same way as in ATHENA IRT codes, where one had an option to build this event
    // structure using reconstructed tracks (in_reco_particles);
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
      particle->SetVertexPosition(Tools::PodioVector3_to_TVector3(mcparticle.getVertex()));
      particle->SetVertexMomentum(Tools::PodioVector3_to_TVector3(mcparticle.getMomentum()));
      particle->SetVertexTime    (mcparticle.getTime());
      
      m_Event->AddChargedParticle(particle);
      MCParticle_to_ChargedParticle[mcid] = particle;

      {
	// FIXME: hardcoded;
	auto aerogel = m_irt_det->GetRadiator("Aerogel");
	
	auto history = new RadiatorHistory();
	particle->StartRadiatorHistory(std::make_pair(aerogel, history));
	
	auto segment = Track_to_TrackSegment_lut[rctrack];//.id().index];
	for(const auto& point : segment.getPoints()) {
	  TVector3 position = Tools::PodioVector3_to_TVector3(point.position);
	  TVector3 momentum = Tools::PodioVector3_to_TVector3(point.momentum);
	  
	  //printf("x=%7.2f y=%7.2f z=%7.2f -> px=%8.4f py=%8.4f pz=%7.2f\n",
	  //	 position.X(), position.Y(), position.Z(), 
	  //	 momentum.X(), momentum.Y(), momentum.Z());
	  
	  auto step = new ChargedParticleStep(position, momentum);
	  history->AddStep(step);
	} //for point
      }
    } //for mcparticle

    // Now loop through simulated hits;
    for(auto mchit: *in_sim_hits) {
      auto cell_id   = mchit.getCellID();
      // Use a fully encoded number in std::map, no tricks;
      uint64_t sensorID = cell_id & m_irt_det->GetReadoutCellMask();
      //printf("@S@ cell: 0x%lX, sensor: 0x%lX\n", cell_id, sensorID);

      //printf("dE: %7.2f\n", 1E9 * mchit.getEDep());
      
      // Get photon which created this hit; filter out charged particles;
      auto &mcparticle = mchit.getMCParticle();
      //printf("%4d -> %d\n", mcparticle.id().index, mcparticle.getPDG());
      if (mcparticle.getPDG() != -22) continue;

      counter++;
      
      // Create an optical photon class instance and populate it; units: [mm], [ns], [eV];
      auto photon = new OpticalPhoton();

      // Information provided by the hit itself: detection position and time;
      photon->SetDetectionPosition(Tools::PodioVector3_to_TVector3(mchit.getPosition()));
      photon->SetDetectionTime    (mchit.getTime());

      // Information inherited from photon MCParticle; FIXME: units?;
      photon->SetVertexPosition   (Tools::PodioVector3_to_TVector3(mcparticle.getVertex()));
      photon->SetVertexMomentum   (1E9 * Tools::PodioVector3_to_TVector3(mcparticle.getMomentum()));
      photon->SetVertexTime       (mcparticle.getTime());

      // FIXME: (0,0,1) or (0,0,-1)?; should be different for e-endcap?;
      TVector3 vtx = Tools::PodioVector3_to_TVector3(mcparticle.getVertex()), n0(0,0,1*sign);
#if 1//_TODAY_
      auto radiator = m_irt_det->GuessRadiator(vtx, n0); 
      //auto radiator = m_irt_det->GetRadiator("Aerogel");//uessRadiator(vtx, n0); 

      auto parents = mcparticle.getParents();
      if (parents.size() != 1) continue;
      unsigned parent_id = mcparticle.parents_begin()->id().index;
      if (MCParticle_to_ChargedParticle.find(parent_id) == MCParticle_to_ChargedParticle.end()) continue;
      auto parent = MCParticle_to_ChargedParticle[parent_id];
	
      if (radiator) {
	//printf("%s -> p: %f\n", m_irt_det->GetRadiatorName(radiator), photon->GetVertexMomentum().Mag());
	
	// FIXME: really needed in ePIC IRT 2.0 implementation?;
	//photon->SetVertexAttenuationLength(GetAttenuationLength(radiator, e));
	{
	  double e = photon->GetVertexMomentum().Mag();
	  double ri = radiator->m_RefractiveIndex->GetInterpolatedValue(e, G4DataInterpolation::FirstOrder);
	  photon->SetVertexRefractiveIndex(ri);
	}
	
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
	// VolumeCopy in a standalone GEANT code, encoded sensor ID in ePIC;
	//photon->SetVolumeCopy(xto->GetTouchable()->GetCopyNumber(pd->GetCopyIdentifierLevel()));
	photon->SetVolumeCopy(sensorID);

	// The logic behind this multiplication and division by the same number is 
	// to select calibration photons, which originate from the same QE(lambda) 
	// parent distribution, but do not pass the overall efficiency test;
	{
	  double e = photon->GetVertexMomentum().Mag();
	  double qe = pd->GetQE()->WithinRange(e) ?
	    pd->GetQE()->GetInterpolatedValue(e, G4DataInterpolation::FirstOrder) : 0.0;
	  
	  if (qe*pd->GetScaleFactor() > m_rngUni()) {
	    if (pd->GetGeometricEfficiency()/pd->GetScaleFactor() > m_rngUni())
	      photon->SetDetected(true);
	    else
	      photon->SetCalibrationFlag();
	  } //if
	}
      } //if

      //? if (!info->Parent()) m_EventPtr->AddOrphanPhoton(photon);
#endif
    } //for mchit
    
    {
      std::lock_guard<std::mutex> lock(m_OutputTreeMutex);
      
      m_EventBranch->SetAddress(m_EventPtr);
      m_EventTree->Fill();
      
      m_Debug->Fill(counter);
    }
  } // IrtDebugging::process()
} // namespace eicrecon

// -------------------------------------------------------------------------------------
