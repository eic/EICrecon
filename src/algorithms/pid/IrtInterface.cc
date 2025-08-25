//
// Copyright 2025, Alexander Kiselev
// Subject to the terms in the LICENSE file found in the top-level directory.
//
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
#include <edm4eic/MutableIrtParticle.h>
#include <edm4eic/MutableIrtRadiator.h>

#include <IRT/ChargedParticle.h>
#include "IRT/CherenkovEvent.h"
#include "IRT/CherenkovDetectorCollection.h"
#include "IRT/ReconstructionFactory.h"

#include "G4DataInterpolation.h"
#include "IrtInterface.h"

// IrtDebugging::process() is const -> move this mutex out for the time being;
static std::mutex m_OutputTreeMutex;

static std::map<std::string, TFile *>   m_OutputFiles;
static std::map<std::string, TTree *>   m_EventTrees;
static std::map<std::string, unsigned>  m_InstanceCounters;
static std::map<std::string, TBranch *> m_EventBranches;

// FIXME: move to a different place;
#define _MAGIC_CFF_                              (1239.8)

using json = nlohmann::json;

// -------------------------------------------------------------------------------------

namespace eicrecon {
  IrtInterface::~IrtInterface()
  {
    std::lock_guard<std::mutex> lock(m_OutputTreeMutex);
      
    //printf("@@@ IrtInterface::~IrtInterface() ... %s %2d %2d\n", m_OutputFileName.c_str(), m_Instance,
    //	   m_InstanceCounters[m_OutputFileName]);
    m_InstanceCounters[m_OutputFileName]--;
    
    if (!m_InstanceCounters[m_OutputFileName]) {
      // FIXME: hardcoded;
      if (m_ReconstructionFactory)
	m_ReconstructionFactory->DisplayStandardPlots("Track / event level plots", -1265,  10,  625,1115);
      
      m_OutputFiles[m_OutputFileName]->cd();
      m_EventTrees[m_OutputFileName]->Write();

      // Write an optics configuration copy into the output event tree; this modified version
      // will in particular contain properly assigned m_ReferenceRefractiveIndex values;
      m_config.m_irt_geometry->Write();
      
      m_OutputFiles[m_OutputFileName]->Close();
    } //if
  } // IrtInterface::~IrtInterface()
  
  // -------------------------------------------------------------------------------------
    
  void IrtInterface::init(DD4hep_service &dd4hep_service, IrtConfig &config,
			  std::shared_ptr<spdlog::logger>& logger)
  {
    //printf("@R@ IrtInterface::init() ...\n");
    
    // FIXME: is this all thread safe?;
    m_config = config;
    
    // FIXME: hardcoded;
    m_random.SetSeed(0x12345678);//m_cfg.seed);
    m_rngUni = [&](){
      return m_random.Uniform(0., 1.0);
    };
    
    m_log = logger;

    // Extract the the relevant `CherenkovDetector`; FIXME: for now assume it is the only one;
    m_irt_det = config.m_irt_geometry->GetDetectors().begin()->second;
    
    {
      std::lock_guard<std::mutex> lock(m_OutputTreeMutex);
    
      m_Event = new CherenkovEvent();
      m_EventPtr = &m_Event;
      
      /*const*/ json *jptr = &config.m_json_config;
      // FIXME: do it better;
      assert(jptr->find("OutputTree") != jptr->end());
      m_OutputFileName = (*jptr)["OutputTree"].template get<std::string>().c_str();

      //+printf("@@@ IrtInterface::init() ... %2d\n", m_InstanceCounters[m_OutputFileName.Data()]);
            
      // FIXME: this is a hack, for the time being;
      if (jptr->find("IntegratedReconstruction") != jptr->end() &&
	  !strcmp((*jptr)["IntegratedReconstruction"].template get<std::string>().c_str(), "yes")) {
	m_ReconstructionFactory = new ReconstructionFactory(config.m_irt_geometry, m_irt_det, m_Event);
	JsonParser();//jptr);
      } //if
    
      if (!m_InstanceCounters[m_OutputFileName]) {
	//printf("@R@ Here %d!\n", m_InstanceCounters[m_OutputFileName]);

	// FIXME: sanity check;
	m_OutputFiles[m_OutputFileName] = new TFile(m_OutputFileName.c_str(), "RECREATE");
	
	m_EventTrees[m_OutputFileName] = new TTree("t", "My tree");
	m_EventBranches[m_OutputFileName] =
	  m_EventTrees[m_OutputFileName]->Branch("e", "CherenkovEvent", 0/*&m_Event*/, 16000, 2);
      } //if

      m_Instance = m_InstanceCounters[m_OutputFileName]++;
    }
    
    {
      const dd4hep::Detector *det = dd4hep_service.detector();

      for(auto [name,rad] : m_irt_det->Radiators()) {
	//printf("@R@ %s\n", name.Data());
	const auto *rindex_matrix = det->material(rad->GetAlternativeMaterialName()).property("RINDEX");
	if (rindex_matrix) {
	  unsigned dim = rindex_matrix->GetRows();
	  double e[dim], ri[dim];
	  for(unsigned row=0; row<rindex_matrix->GetRows(); row++) {
	    e [row] = rindex_matrix->Get(row,0) / dd4hep::eV;
	    ri[row] = rindex_matrix->Get(row,1);
	    
	    //printf("%7.3f %7.3f\n", energy, rindex);
	  } //for row
	  
	  auto ptr = rad->m_RefractiveIndex = new G4DataInterpolation(e, ri, dim);
	  // FIXME: 100 hardcoded;
	  ptr->CreateLookupTable(100);
	} //if
      } //for radiators
    }
    
    {
      /*const*/ json *jptr = &config.m_json_config;

      // FIXME: for now assume a single photo detector type; cannot easily store this pointer;
      auto pd = m_irt_det->m_PhotonDetectors[0];

      if (jptr->find("Photosensor") != jptr->end()) {
	/*const*/ auto &jpref = (*jptr)["Photosensor"];
	
	double qe_rescaling_factor = 1.0;
	// An artificial rescaling factor may be provided;
	if (jpref.find("quantum-efficiency-rescaling-factor") != jpref.end()) 
	  qe_rescaling_factor = jpref["quantum-efficiency-rescaling-factor"].template get<double>();
	
	if (jpref.find("quantum-efficiency") != jpref.end()) {
	  /*const*/ auto &qeref = jpref["quantum-efficiency"];
	  
	  const unsigned qeEntries = qeref.size();
	  double WL[qeEntries], QE[qeEntries];
	  
	  unsigned counter = 0;
	  for (json::iterator it = qeref.begin(); it != qeref.end(); ++it) {
	    std::string wlstr(it.key().c_str());
	    // FIXME: assumes a 3-digit integer value; do it better later;
	    wlstr[3] = 0;
	    
	    WL[counter  ] = atoi(wlstr.c_str());
	    QE[counter++] = it.value(). template get<double>();
	    //printf("%7.2f %7.2f\n", WL[counter-1], QE[counter-1]);
	  } //it
	  
	  double qemax = 0.0, qePhotonEnergy[qeEntries], qeData[qeEntries];
	  for(int iq=0; iq<qeEntries; iq++) {
	    qePhotonEnergy[iq] = _MAGIC_CFF_ / (WL[qeEntries - iq - 1] + 0.0);
	    qeData        [iq] =                QE[qeEntries - iq - 1] * qe_rescaling_factor;
	    
	    if (qeData[iq] > qemax) qemax = qeData[iq];
	  } //for iq
	  
	  pd->SetQE(_MAGIC_CFF_ / WL[qeEntries-1], _MAGIC_CFF_ / WL[0], 
		    // NB: last argument: want a built-in selection of unused photons, which follow the QE(lambda);
		    // see CherenkovSteppingAction::UserSteppingAction() for a usage case;
		    new G4DataInterpolation(qePhotonEnergy, qeData, qeEntries/*, 0.0, 0.0*/), qemax ? 1.0/qemax : 1.0);
	  // FIXME: 100 hardcoded;
	  pd->GetQE()->CreateLookupTable(100);
	} //if
      } //if
    }
    
    //printf("@R@ IrtInterface::init() finished ...\n");
  } // IrtInterface::init()

  // -------------------------------------------------------------------------------------
  
  void IrtInterface::process(
			     const IrtInterface::Input& input,
			     const IrtInterface::Output& output
			     ) const
  {
    //printf("@R@ IrtInterface::process() ...\n");

    // Reset output event structure;
    m_Event->Reset();

    // Intermediate variables, for less typing;
    const auto [in_mc_particles,
		in_reco_particles,
		in_mc_reco_associations,
		in_track_projections, in_sim_hits] = input;
    auto [out_irt_radiator_info, out_irt_particles, out_irt_event] = output;

    //auto irtEvent = out_irt_event->create();
      
    // First build MC->reco lookup table;
    std::map<unsigned, std::vector<unsigned>> MCParticle_to_Tracks_lut;
    for(const auto &assoc: *in_mc_reco_associations) {
      // MC particle index in its respective in_mc_particles array;
      unsigned mcid = assoc.getSimID();//.id().index;
      // Reco particle index in its respective in_reco_particles array;
      unsigned rcid = assoc.getRecID();//.id().index;
    
      auto rcparticle = (*in_reco_particles)[rcid];//assoc.getRecID()];
      //auto tnum = rcparticle.getTracks().size();
      //printf("tnum: %ld\n", rcparticle.getTracks().size());
      for(auto &track: rcparticle.getTracks())
	MCParticle_to_Tracks_lut[mcid].push_back(track.id().index);
    } //for assoc

    // Then track -> track projection lookup table; FIXME: other radiators; 
    std::map<unsigned, edm4eic::TrackSegment> Track_to_TrackSegment_lut;
    //printf("track projection group(s): %ld\n", (*in_track_projections).size());
    for(auto segment: *in_track_projections) {
      auto track = segment.getTrack();
      //printf("(3)   --> %d\n", track.id().index);
      Track_to_TrackSegment_lut[track.id().index] = segment;
    } //for particle
    
    // Help optical photons to find their parents;
    std::map<unsigned, ChargedParticle*> MCParticle_to_ChargedParticle;
        
    // Create event structure a la standalone pfRICH/IRT code; use MC particles (in_mc_particles)
    // in this first iteration (and only select primary ones); later on should do it probably
    // the same way as in ATHENA IRT codes, where one had an option to build this event
    // structure using reconstructed tracks (in_reco_particles);
    for(const auto &mcparticle: *in_mc_particles) {
      // Deal only with charged primary ones, for the time being; FIXME: low momentum cutoff?;
      if (mcparticle.isCreatedInSimulation() || !mcparticle.getCharge()) continue;
      //printf("charge: %f\n", mcparticle.getCharge());

      unsigned mcid = mcparticle.id().index;
      
      // Now check that MC->reco association exists; for now ignore cases where more than one
      // reconstructed track is associated with a given MC particle;
      if (MCParticle_to_Tracks_lut.find(mcid) == MCParticle_to_Tracks_lut.end()) continue;
      auto &rctracks = MCParticle_to_Tracks_lut[mcid];
      if (rctracks.size() > 1) continue;
      auto &rctrack = rctracks[0];

      // Do not want to deal with particles outside of the nominal acceptance; FIXME: do it better later;
      {
	double eta = Tools::PodioVector3_to_TVector3(mcparticle.getMomentum()).Eta();
	if (eta < m_config.m_eta_min || eta > m_config.m_eta_max) continue; 
      }

      // Now add a charged particle to the event structure; 'true': primary;
      auto particle = new ChargedParticle(mcparticle.getPDG(), true);
      
      // For now, just record a reference to EICrecon track; the actual loop with assignments
      // will be at the end of processing;
      particle->SetEICreconParticleID(rctrack);
      
      // FIXME: check units;
      particle->SetVertexPosition(Tools::PodioVector3_to_TVector3(mcparticle.getVertex()));
      particle->SetVertexMomentum(Tools::PodioVector3_to_TVector3(mcparticle.getMomentum()));
      particle->SetVertexTime    (mcparticle.getTime());
      
      m_Event->AddChargedParticle(particle);
      MCParticle_to_ChargedParticle[mcid] = particle;

      // Create history records for all known radiators; FIXME: may want to optimize a bit;
      for(auto [name,rad] : m_irt_det->Radiators()) {
	auto history = new RadiatorHistory();
	particle->StartRadiatorHistory(std::make_pair(rad, history));
      } //for radiator

      // Record track projections; FIXME: do it only for radiators used for imaging?;
      {
	auto segment = Track_to_TrackSegment_lut[rctrack];//.id().index];
	//printf("--> %4ld\n", segment.getPoints().size());
	for(const auto& point : segment.getPoints()) {
	  TVector3 position = Tools::PodioVector3_to_TVector3(point.position);
	  TVector3 momentum = Tools::PodioVector3_to_TVector3(point.momentum);
	  
	  //printf("x=%7.2f y=%7.2f z=%7.2f -> px=%8.4f py=%8.4f pz=%7.2f\n",
	  //	 position.X(), position.Y(), position.Z(), 
	  //	 momentum.X(), momentum.Y(), momentum.Z());

	  // FIXME: this call is relatively CPU-intensive; may want to create a lookup
	  // table, since in principle it is known which projection point corresponds to
	  // which radiator; however, this way would not be exactly clean because of
	  // spherical boundaries; leave as it is for now and optimize later;
	  auto radiator = m_irt_det->GuessRadiator(position, momentum.Unit());
	  if (radiator) {
	    auto history = particle->FindRadiatorHistory(radiator);

	    // FIXME: this check is redundant?;
	    if (history) {
	      auto step = new ChargedParticleStep(position, momentum);
	      history->AddStep(step);
	    } //if
	  } //if
	} //for point
      }
    } //for mcparticle

    // Now loop through simulated hits;
    //printf("--> %4ld hits\n", (*in_sim_hits).size());
    for(auto mchit: *in_sim_hits) {
      auto cell_id   = mchit.getCellID();
      // Use a fully encoded number in std::map, no tricks;
      uint64_t sensorID = cell_id & m_irt_det->GetReadoutCellMask();
      //printf("@S@ cell: 0x%lX, sensor: 0x%lX\n", cell_id, sensorID);

      //printf("dE: %7.2f\n", 1E9 * mchit.getEDep());

      //printf("Here-A\n");
      // Get photon which created this hit; filter out charged particles;
      auto const &mcparticle = mchit.getParticle();//MCParticle();
      //printf("%4d -> %d\n", mcparticle.id().index, mcparticle.getPDG());
      if (mcparticle.getPDG() != -22) continue;

      //printf("Here-B\n");
      //counter++;
      
      // Create an optical photon class instance and populate it; units: [mm], [ns], [eV];
      auto photon = new OpticalPhoton();

      // Information provided by the hit itself: detection position and time;
      photon->SetDetectionPosition(Tools::PodioVector3_to_TVector3(mchit.getPosition()));
      photon->SetDetectionTime    (mchit.getTime());

      // Information inherited from photon MCParticle; FIXME: units?;
      photon->SetVertexPosition   (Tools::PodioVector3_to_TVector3(mcparticle.getVertex()));
      photon->SetVertexMomentum   (1E9 * Tools::PodioVector3_to_TVector3(mcparticle.getMomentum()));
      photon->SetVertexTime       (mcparticle.getTime());
      
      auto parents = mcparticle.getParents();
      //printf("   --> %4ld parents\n", parents.size());
      if (parents.size() != 1) continue;
      //printf("Here-C\n");
      unsigned parent_id = mcparticle.parents_begin()->id().index;
      //printf("   --> %4d\n", MCParticle_to_ChargedParticle.find(parent_id) == MCParticle_to_ChargedParticle.end());
      if (MCParticle_to_ChargedParticle.find(parent_id) == MCParticle_to_ChargedParticle.end()) continue;
      auto parent = MCParticle_to_ChargedParticle[parent_id];
      //printf("Here-D\n");

      TVector3 vtx = Tools::PodioVector3_to_TVector3(mcparticle.getVertex());
      // FIXME: may want to use the very first projection rather than the IP info?;
      auto radiator = m_irt_det->GuessRadiator(vtx, parent->GetVertexMomentum().Unit());
      //printf("   %7.2f -> %p\n", vtx.Z(), radiator);
      
      if (radiator) {
	// FIXME: really needed in ePIC IRT 2.0 implementation?;
	//photon->SetVertexAttenuationLength(GetAttenuationLength(radiator, e));
	{
	  double e = photon->GetVertexMomentum().Mag();
	  double ri = radiator->m_RefractiveIndex->GetInterpolatedValue(e, G4DataInterpolation::FirstOrder);
	  photon->SetVertexRefractiveIndex(ri);

	  // Will be stored in a (fixed) order in which radiators were defined for this Cherenkov detector;
	  for(auto [name,rad] : m_irt_det->Radiators())
	    photon->StoreRefractiveIndex(rad->m_RefractiveIndex->GetInterpolatedValue(e, G4DataInterpolation::FirstOrder));
	}
	
	auto history = parent->FindRadiatorHistory(radiator);
	// FIXME: this check is redundant?;
	if (history) history->AddOpticalPhoton(photon);
      } else {
	//+if (m_irt_det_coll->CheckBits(_STORE_ORPHAN_PHOTONS_))
	parent->AddOrphanPhoton(photon);
      } //if

      // FIXME: this is kind of a hack (assume a single type of photodetectors); should be fine
      // for ePIC, though a standalone GEANT code has amore generic implementation;
      if (m_irt_det->m_PhotonDetectors.size() != 1) continue;
      auto pd = m_irt_det->m_PhotonDetectors[0];
      
      if (pd) {
	photon->SetPhotonDetector(pd);
	// Would be a VolumeCopy in a standalone GEANT code, but is an encoded sensor ID in ePIC;
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
    } //for mchit

#if 0
    for(auto mcparticle: m_Event->ChargedParticles()) {
      unsigned npe_per_track = 0;//, nhits_per_track = 0;

      //printf("   --> %4ld radiators\n", mcparticle->GetRadiatorHistory().size());
      for(auto rhptr: mcparticle->GetRadiatorHistory()) {
	auto radiator = mcparticle->GetRadiator(rhptr);
	//if (!radiator->UsedInRingImaging()) continue;
	
	unsigned npe_per_radiator = 0;//, nhits_per_radiator = 0;
	
	//nhits_per_radiator = mcparticle->GetRecoCherenkovPhotonCount(radiator);
	//nhits_per_track += nhits_per_radiator;
	
	//auto *plots = radiator->Plots();
	  
	for(auto photon: mcparticle->GetHistory(rhptr)->Photons()) 
	  if (photon->WasDetected()) 
	    npe_per_radiator++;

	//printf("     --> %4d detected photons\n", npe_per_radiator);
      } //for rhistory
    } //for mcparticle
#endif
	    
    {
      std::lock_guard<std::mutex> lock(m_OutputTreeMutex);
      
      m_EventBranches[m_OutputFileName]->SetAddress(m_EventPtr);
      m_EventTrees[m_OutputFileName]->Fill();
    }

    // FIXME: this is a hack to the moment;
    if (m_ReconstructionFactory) m_ReconstructionFactory->GetEvent(0, false);

    // And eventually, populate PODIO output tables;
    {
      out_irt_radiator_info->create();
      /*auto irtParticles =*/ out_irt_particles->create();
      auto irtEvent = out_irt_event->create();
    
      for(auto particle: m_Event->ChargedParticles()) {
	if (!particle->IsPrimary()) continue;
	
	unsigned npe_per_track = 0, nhits_per_track = 0;

	edm4eic::MutableIrtParticle irtParticle;
	//printf("--> %d\n", particle->m_EICreconParticleID);
	irtParticle.setChargedParticle((*in_reco_particles)[particle->m_EICreconParticleID]);
	
	//printf("   --> %4ld radiators\n", mcparticle->GetRadiatorHistory().size());
	for(auto rhptr: particle->GetRadiatorHistory()) {
	  auto radiator = particle->GetRadiator(rhptr);
	  if (!radiator->UsedInRingImaging()) continue;
	  
	  edm4eic::MutableIrtRadiatorInfo irtRadiator;
	  unsigned npe_per_radiator = 0, nhits_per_radiator = 0;
	  
	  nhits_per_radiator = particle->GetRecoCherenkovPhotonCount(radiator);
	  irtRadiator.setNhits(nhits_per_radiator);
	  nhits_per_track += nhits_per_radiator;
	  
	  for(auto photon: particle->GetHistory(rhptr)->Photons()) 
	    if (photon->WasDetected()) 
	      npe_per_radiator++;
	  
	  //printf("     --> %4d detected photons\n", npe_per_radiator);
	  irtRadiator.setNpe(npe_per_radiator);
	  npe_per_track += npe_per_radiator;

	  irtRadiator.setAngle(1000*particle->GetRecoCherenkovAverageTheta(radiator));
	  
	  out_irt_radiator_info->push_back(irtRadiator);
	  irtParticle.addToRadiators(irtRadiator);
	} //for rhistory

	irtParticle.setPDG  (particle->GetPDG());
	irtParticle.setNpe  (  npe_per_track);
	irtParticle.setNhits(nhits_per_track);
	
	out_irt_particles->push_back(irtParticle);
	irtEvent.addToIrtParticles(irtParticle);
      } //for particle
    }
  } // IrtInterface::process()
} // namespace eicrecon

// -------------------------------------------------------------------------------------

