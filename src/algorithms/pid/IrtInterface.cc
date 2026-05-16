//
// Copyright 2025, Alexander Kiselev
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifdef WITH_IRT2_SUPPORT

#include <DD4hep/Detector.h>
#include <DD4hep/Objects.h>
#include <Evaluator/DD4hepUnits.h>
#include <IRT2/ChargedParticle.h>
#include <IRT2/ChargedParticleStep.h>
#include <IRT2/CherenkovDetector.h>
#include <IRT2/CherenkovDetectorCollection.h>
#include <IRT2/CherenkovEvent.h>
#include <IRT2/CherenkovPhotonDetector.h>
#include <IRT2/CherenkovRadiator.h>
#include <IRT2/OpticalPhoton.h>
#include <IRT2/RadiatorHistory.h>
#include <IRT2/ReconstructionFactory.h>
#include <TBranch.h>
#include <TFile.h>
#include <TGDMLMatrix.h>
#include <TString.h>
#include <TTree.h>
#include <TVector3.h>
#include <assert.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackPoint.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <cctype>
#include <gsl/pointers>
#include <map>
//#include <mutex>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

#include "algorithms/pid/IrtInterfaceConfig.h"
#include "algorithms/pid/Tools.h"

using namespace IRT2;

#include "G4DataInterpolation.h"
#include "IrtInterface.h"

// IrtDebugging::process() is const -> move this mutex out for the time being;
//static std::mutex m_OutputTreeMutex;

//static std::map<std::string, TFile*> m_OutputFiles;
//static std::map<std::string, TTree*> m_EventTrees;
//static std::map<std::string, unsigned> m_InstanceCounters;
//static std::map<std::string, TBranch*> m_EventBranches;

// FIXME: move to a different place;
#define _MAGIC_CFF_ (1239.8)

using json = nlohmann::json;

// -------------------------------------------------------------------------------------

#include <TApplication.h>
#include <TCanvas.h>

namespace eicrecon {
IrtInterface::~IrtInterface() {
  printf("@Q@ IrtInterface::~IrtInterface() ...\n");
  //  std::lock_guard<std::mutex> lock(m_OutputTreeMutex);

  //m_InstanceCounters[m_OutputFileName]--;

  //printf("@Q@ Here-0 : %d\n", m_InstanceCounters[m_OutputFileName]);
  //+if (!m_InstanceCounters[m_OutputFileName]) {
  //if (m_InstanceCounters[m_OutputFileName] == 1) {
  //printf("@Q@ Here-1\n");
  //#if _MOVED_
  //m_OutputFiles[m_OutputFileName]->cd();
  //if (m_EventTreeOutputEnabled)
  //  m_EventTrees[m_OutputFileName]->Write();
  //#endif

  // Avoid calling this stuff from a dummy IrtInterface instantiation upon eicrecon startup;
  if (*m_ProcessedEventsPtr) {
    if (m_OutputFile) {
      m_OutputFile->cd();

      if (m_EventTreeOutputEnabled)
        m_EventTree->Write();
    } //if

    // Avoid calling this stuff from a dummy IrtInterface instantiation upon eicrecon startup;
    if (m_ReconstructionFactory && m_ReconstructionFactory->GetProcessedEventCount()) {
      printf("@Q@ Here-3\n");

      int argc      = 1;
      char* argv[1] = {(char*)""};
      bool display  = m_CombinedPlotVisualizationEnabled;
      for (auto [name, rad] : m_cfg.m_irt_detector->Radiators())
        if (rad->UsedInRingImaging() && rad->m_OutputPlotVisualizationEnabled)
          display = true;

      // FIXME: well, if at least one is "display", all "store" will be shown as well;
      auto* app = display ? new TApplication("", &argc, argv) : 0;

      // std::vector<std::pair<TCanvas*, bool>> canvases;
      std::vector<TCanvas*> canvases;
      auto cv = m_ReconstructionFactory->DisplayStandardPlots("Track / event level plots", m_wtopx,
                                                              m_wtopy, m_wx, m_wy);
      if (cv)
        canvases.push_back(cv);

      for (auto [name, rad] : m_cfg.m_irt_detector->Radiators())
        if (rad->UsedInRingImaging()) {
          TString cname, wname;
          // FIXME: won't work for Acrylic and Aerogel together;
          cname.Form("c%c", std::tolower(name.Data()[0]));
          wname.Form("%s radiator", name.Data());

          auto cv = rad->DisplayStandardPlots(cname.Data(), wname.Data(),
                                              // FIXME: may want to improve the API here;
                                              rad->m_wtopx, rad->m_wtopy, rad->m_wx, rad->m_wy);
          if (cv)
            canvases.push_back(cv);
        } //for rad..if

      // 'true': do not call exit() in the end;
      if (app && canvases.size())
        app->Run(true);
      // FIXME: crashes;
      //delete app;

      if (m_OutputFile)
        for (auto cv : canvases)
          cv->Write();
    } //if

    // Write an optics configuration copy into the output event tree; this modified version
    // will in particular contain properly assigned m_ReferenceRefractiveIndex values;
    // FIXME: needs to be written out even if m_ReconstructionFactory=0;
    if (m_OutputFile) {
      m_cfg.m_irt_geometry->Write();
      m_OutputFile->Close();

      m_OutputFile = 0;
    } //if

    delete m_ReconstructionFactory;
    m_ReconstructionFactory = 0;
  } //if

#if _MOVED_
  // Write an optics configuration copy into the output event tree; this modified version
  // will in particular contain properly assigned m_ReferenceRefractiveIndex values;
  m_cfg.m_irt_geometry->Write();
#endif

  //if (m_EventTreeOutputEnabled) {
  //m_EventTrees[m_OutputFileName] = new TTree("t", "IRT2 output tree");
  //m_EventBranches[m_OutputFileName] =
  //    m_EventTrees[m_OutputFileName]->Branch("e", "CherenkovEvent", 0, 16000, 2);
  //} //if
  //} //if

  //m_Instance = m_InstanceCounters[m_OutputFileName]++;
}

void IrtInterface::init() {
  printf("@Q@ IrtInterface::init() ...\n");
  // FIXME: hardcoded;
  m_random.SetSeed(0x12345678); //m_cfg.seed);
  m_rngUni = [&]() { return m_random.Uniform(0., 1.0); };

  // Extract the relevant `CherenkovDetector`; FIXME: for now assume it is the only one;
  //m_irt_det = m_cfg.m_irt_geometry->GetDetectors().begin()->second;

  {
    //std::lock_guard<std::mutex> lock(m_OutputTreeMutex);

    m_Event    = new IRT2::CherenkovEvent();
    m_EventPtr = &m_Event;

    json* jptr = &m_cfg.m_json_config;
    // FIXME: do it better;
#if 1
    if (jptr->find("OutputRootFile") != jptr->end())
      m_OutputFileName = (*jptr)["OutputRootFile"].template get<std::string>().c_str();
#else
    assert(jptr->find("OutputRootFile") != jptr->end());
    m_OutputFileName = (*jptr)["OutputRootFile"].template get<std::string>().c_str();
#endif

    // FIXME: this is a hack, for the time being;
    if (jptr->find("IntegratedReconstruction") != jptr->end() &&
        !strcmp((*jptr)["IntegratedReconstruction"].template get<std::string>().c_str(), "yes")) {
      m_ReconstructionFactory =
          new IRT2::ReconstructionFactory(m_cfg.m_irt_geometry, m_cfg.m_irt_detector, m_Event);
      // JANA2 prints out event progress; the rest is kind of irrelevant;
      m_ReconstructionFactory->SetQuietMode();
      // FIXME: add syntax check and return value;
      JsonParser();
    } //if

    if (jptr->find("WriteOutputTree") != jptr->end() &&
        !strcmp((*jptr)["WriteOutputTree"].template get<std::string>().c_str(), "no"))
      m_EventTreeOutputEnabled = false;

    //if (!m_InstanceCounters[m_OutputFileName]) {
    if (!m_OutputFile && m_OutputFileName.ends_with(".root")) {
      // FIXME: sanity check;
      m_OutputFile = new TFile(m_OutputFileName.c_str(), "RECREATE");

      if (m_EventTreeOutputEnabled) {
        m_EventTree   = new TTree("t", "IRT2 output tree");
        m_EventBranch = m_EventTree->Branch("e", "CherenkovEvent", m_EventPtr, 16000, 2);
      } //if
    } //if

    //m_Instance = m_InstanceCounters[m_OutputFileName]++;
  }

  {
    const dd4hep::Detector* det = m_geo.detector();

    for (auto [name, rad] : m_cfg.m_irt_detector->Radiators()) {
      const auto* rindex_matrix =
          det->material(rad->GetAlternativeMaterialName()).property("RINDEX");
      if (rindex_matrix) {
        const unsigned dim = rindex_matrix->GetRows();
        std::unique_ptr<double[]> e(new double[dim]);
        std::unique_ptr<double[]> ri(new double[dim]);

        for (unsigned row = 0; row < rindex_matrix->GetRows(); row++) {
          e[row]  = rindex_matrix->Get(row, 0) / dd4hep::eV;
          ri[row] = rindex_matrix->Get(row, 1);
        } //for row

        auto ptr = rad->m_RefractiveIndex = new G4DataInterpolation(e.get(), ri.get(), dim);
        // FIXME: 100 hardcoded;
        ptr->CreateLookupTable(100);
      } //if
    } //for radiators
  }

  {
    json* jptr = &m_cfg.m_json_config;

    // FIXME: for now assume a single photo detector type; cannot easily store this pointer;
    auto pd = m_cfg.m_irt_detector->m_PhotonDetectors[0];

    if (jptr->find("Photosensor") != jptr->end()) {
      auto& jpref = (*jptr)["Photosensor"];

      double qe_rescaling_factor = 1.0;
      // An artificial rescaling factor may be provided;
      if (jpref.find("quantum-efficiency-rescaling-factor") != jpref.end())
        qe_rescaling_factor = jpref["quantum-efficiency-rescaling-factor"].template get<double>();

      if (jpref.find("quantum-efficiency") != jpref.end()) {
        auto& qeref = jpref["quantum-efficiency"];

        const int qeEntries = qeref.size();
        std::unique_ptr<double[]> WL(new double[qeEntries]);
        std::unique_ptr<double[]> QE(new double[qeEntries]);

        unsigned counter = 0;
        for (json::iterator it = qeref.begin(); it != qeref.end(); ++it) {
          std::string wlstr(it.key().c_str());
          // FIXME: assumes a 3-digit integer value; do it better later;
          wlstr[3] = 0;

          WL[counter]   = atoi(wlstr.c_str());
          QE[counter++] = it.value().template get<double>();
        } //it

        double qemax = 0.0;
        std::vector<double> qePhotonEnergy(qeEntries);
        std::vector<double> qeData(qeEntries);
        for (int iq = 0; iq < qeEntries; iq++) {
          qePhotonEnergy[iq] = _MAGIC_CFF_ / (WL[qeEntries - iq - 1] + 0.0);
          qeData[iq]         = QE[qeEntries - iq - 1] * qe_rescaling_factor;

          if (qeData[iq] > qemax)
            qemax = qeData[iq];
        } //for iq

        pd->SetQE(
            _MAGIC_CFF_ / WL[qeEntries - 1], _MAGIC_CFF_ / WL[0],
            // NB: last argument: want a built-in selection of unused photons, which follow the QE(lambda);
            // see CherenkovSteppingAction::UserSteppingAction() for a usage case;
            new G4DataInterpolation(qePhotonEnergy.data(), qeData.data(), qeEntries),
            qemax ? 1.0 / qemax : 1.0);
        // FIXME: 100 hardcoded;
        pd->GetQE()->CreateLookupTable(100);
      } //if
    } //if
  }
} // IrtInterface::init()

// -------------------------------------------------------------------------------------

void IrtInterface::process(const IrtInterface::Input& input,
                           const IrtInterface::Output& output) const {
  printf("IrtInterface::process() ...\n");

  (*m_ProcessedEventsPtr)++;

  // Reset output event structure;
  m_Event->Reset();

  // Intermediate variables, for less typing;
  const auto [in_mc_particles, in_tracks, in_track_associations, in_track_projections,
              in_sim_hits]                        = input;
  auto [out_irt_radiator_info, out_irt_particles] = output;

  // First build MC->reco lookup table;
  std::map<unsigned, std::vector<unsigned>> MCParticle_to_Tracks_lut;
  printf("in_track_associations size: %ld\n", (*in_track_associations).size());
  for (const auto& assoc : *in_track_associations) {
    // MC particle index in its respective in_mc_particles array;
    unsigned mcid = assoc.getSim().getObjectID().index;
    // Reco particle index in its respective in_tracks array;
    unsigned rcid = assoc.getRec().getObjectID().index;

    auto track = (*in_tracks)[rcid];
    MCParticle_to_Tracks_lut[mcid].push_back(track.id().index);
  } //for assoc

  // Then track -> track projection lookup table; FIXME: other radiators;
  std::map<unsigned, edm4eic::TrackSegment> Track_to_TrackSegment_lut;
  printf("in_track_projections size: %ld\n", (*in_track_projections).size());
  for (auto segment : *in_track_projections) {
    auto track = segment.getTrack();

    Track_to_TrackSegment_lut[track.id().index] = segment;
  } //for particle

  // Help optical photons to find their parents;
  std::map<unsigned, ChargedParticle*> MCParticle_to_ChargedParticle;

  // Create event structure a la standalone pfRICH/IRT code; use MC particles (in_mc_particles)
  // in this first iteration (and only select primary ones); later on should do it probably
  // the same way as in ATHENA IRT codes, where one had an option to build this event
  // structure using reconstructed tracks (in_tracks);
  printf("in_mc_particles size: %ld\n", (*in_mc_particles).size());
  for (const auto& mcparticle : *in_mc_particles) {
    //printf("Here-1 (%d, %7.1f)\n", mcparticle.isCreatedInSimulation(), mcparticle.getCharge());
    // Deal only with charged primary ones, for the time being; FIXME: low momentum cutoff?;
    if (mcparticle.isCreatedInSimulation() || !mcparticle.getCharge())
      continue;

    //printf("Here-2\n");
    unsigned mcid = mcparticle.id().index;

    // Now check that MC->reco association exists; for now ignore cases where more than one
    // reconstructed track is associated with a given MC particle;
    if (MCParticle_to_Tracks_lut.find(mcid) == MCParticle_to_Tracks_lut.end())
      continue;
    //printf("Here-3\n");
    auto& rctracks = MCParticle_to_Tracks_lut[mcid];
    if (rctracks.size() > 1)
      continue;
    //printf("Here-4\n");
    unsigned rctrack = rctracks[0];

    // Do not want to deal with particles outside of the nominal acceptance; FIXME: do it better later;
    {
      double eta = Tools::PodioVector3_to_TVector3(mcparticle.getMomentum()).Eta();
      if (eta < m_cfg.m_eta_min || eta > m_cfg.m_eta_max)
        continue;
    }

    //printf("Here-5\n");
    // Now add a charged particle to the event structure; 'true': primary;
    auto particle = new ChargedParticle(mcparticle.getPDG(), true);

    // For now, just record a reference to EICrecon track; the actual loop with assignments
    // will be at the end of processing;
    particle->SetEICreconParticleID(rctrack);

    // FIXME: check units;
    particle->SetVertexPosition(Tools::PodioVector3_to_TVector3(mcparticle.getVertex()));
    particle->SetVertexMomentum(Tools::PodioVector3_to_TVector3(mcparticle.getMomentum()));
    particle->SetVertexTime(mcparticle.getTime());

    m_Event->AddChargedParticle(particle);
    MCParticle_to_ChargedParticle[mcid] = particle;

    // Create history records for all known radiators; FIXME: may want to optimize a bit;
    for (auto [name, rad] : m_cfg.m_irt_detector->Radiators()) {
      auto history = new RadiatorHistory();
      particle->StartRadiatorHistory(std::make_pair(rad, history));
    } //for radiator

    // Record track projections; FIXME: do it only for radiators used for imaging?;
    {
      auto segment = Track_to_TrackSegment_lut[rctrack];

      for (const auto& point : segment.getPoints()) {
        TVector3 position = Tools::PodioVector3_to_TVector3(point.position);
        TVector3 momentum = Tools::PodioVector3_to_TVector3(point.momentum);

        // FIXME: this call is relatively CPU-intensive; may want to create a lookup
        // table, since in principle it is known which projection point corresponds to
        // which radiator; however, this way would not be exactly clean because of
        // spherical boundaries; leave as it is for now and optimize later;
        auto radiator = m_cfg.m_irt_detector->GuessRadiator(position, momentum.Unit());
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
  printf("in_sim_hits size: %ld\n", (*in_sim_hits).size());
  for (auto mchit : *in_sim_hits) {
    auto cell_id      = mchit.getCellID();
    uint64_t sensorID = cell_id & m_cfg.m_irt_detector->GetReadoutCellMask();

    // Get photon which created this hit; filter out charged particles;
    auto const& mcparticle = mchit.getParticle();
    if (mcparticle.getPDG() != -22)
      continue;

    //printf("Here-P1\n");
    // Create an optical photon class instance and populate it; units: [mm], [ns], [eV];
    auto photon = new IRT2::OpticalPhoton();

    // Information provided by the hit itself: detection position and time;
    photon->SetDetectionPosition(Tools::PodioVector3_to_TVector3(mchit.getPosition()));
    photon->SetDetectionTime(mchit.getTime());

    // Information inherited from photon MCParticle; FIXME: units?;
    photon->SetVertexPosition(Tools::PodioVector3_to_TVector3(mcparticle.getVertex()));
    photon->SetVertexMomentum(1E9 * Tools::PodioVector3_to_TVector3(mcparticle.getMomentum()));
    photon->SetVertexTime(mcparticle.getTime());

    auto parents = mcparticle.getParents();
    if (parents.size() != 1)
      continue;
    //printf("Here-P2\n");
    unsigned parent_id = mcparticle.parents_begin()->id().index;
    if (MCParticle_to_ChargedParticle.find(parent_id) == MCParticle_to_ChargedParticle.end())
      continue;
    //printf("Here-P3\n");
    auto parent = MCParticle_to_ChargedParticle[parent_id];

    TVector3 vtx = Tools::PodioVector3_to_TVector3(mcparticle.getVertex());
    // FIXME: may want to use the very first projection rather than the IP info?;
    auto radiator = m_cfg.m_irt_detector->GuessRadiator(vtx, parent->GetVertexMomentum().Unit());

    if (radiator) {
      //printf("Here-P4\n");
      {
        double e = photon->GetVertexMomentum().Mag();
        double ri =
            radiator->m_RefractiveIndex->GetInterpolatedValue(e, G4DataInterpolation::FirstOrder);
        photon->SetVertexRefractiveIndex(ri);

        // Will be stored in a (fixed) order in which radiators were defined for this Cherenkov detector;
        for (auto [name, rad] : m_cfg.m_irt_detector->Radiators())
          photon->StoreRefractiveIndex(
              rad->m_RefractiveIndex->GetInterpolatedValue(e, G4DataInterpolation::FirstOrder));
      }

      auto history = parent->FindRadiatorHistory(radiator);
      // FIXME: this check is redundant?;
      if (history) {
        //printf("Here-P5\n");
        history->AddOpticalPhoton(photon);
      } //if
    } else {
      parent->AddOrphanPhoton(photon);
    } //if

    // FIXME: this is kind of a hack (assume a single type of photodetectors); should be fine
    // for ePIC, though a standalone GEANT code has amore generic implementation;
    if (m_cfg.m_irt_detector->m_PhotonDetectors.size() != 1)
      continue;
    auto pd = m_cfg.m_irt_detector->m_PhotonDetectors[0];

    if (pd) {
      //printf("Here-P6\n");
      photon->SetPhotonDetector(pd);
      // Would be a VolumeCopy in a standalone GEANT code, but is an encoded sensor ID (with
      // a blanked out 'sector' field for dRICH) in ePIC;
      photon->SetVolumeCopy(sensorID);

      // The logic behind this multiplication and division by the same number is
      // to select calibration photons, which originate from the same QE(lambda)
      // parent distribution, but do not pass the overall efficiency test;
      {
        double e  = photon->GetVertexMomentum().Mag();
        double qe = pd->GetQE()->WithinRange(e)
                        ? pd->GetQE()->GetInterpolatedValue(e, G4DataInterpolation::FirstOrder)
                        : 0.0;

        if (qe * pd->GetScaleFactor() > m_rngUni()) {
          if (pd->GetGeometricEfficiency() / pd->GetScaleFactor() > m_rngUni())
            photon->SetDetected(true);
          else
            photon->SetCalibrationFlag();
        } //if
      }
    } //if

    // FIXME: should be added? if (!info->Parent()) m_EventPtr->AddOrphanPhoton(photon);
  } //for mchit

  if (m_EventTreeOutputEnabled) {
    //std::lock_guard<std::mutex> lock(m_OutputTreeMutex);

    //m_EventBranches[m_OutputFileName]->SetAddress(m_EventPtr);
    m_EventTree->Fill();
  }

  // FIXME: this is a hack to the moment;
  if (m_ReconstructionFactory)
    m_ReconstructionFactory->GetEvent(0, false);

  // And eventually, populate PODIO output tables;
  {
    out_irt_radiator_info->create();
    out_irt_particles->create();

    for (auto particle : m_Event->ChargedParticles()) {
      if (!particle->IsPrimary())
        continue;

      printf("Here-Q1\n");
      unsigned npe_per_track = 0, nhits_per_track = 0;

      edm4eic::MutableIrtParticle irtParticle;
      irtParticle.setTrack((*in_tracks)[particle->m_EICreconParticleID]);

      for (auto rhptr : particle->GetRadiatorHistory()) {
        auto radiator = particle->GetRadiator(rhptr);
        if (!radiator->UsedInRingImaging())
          continue;

        edm4eic::MutableIrtRadiatorInfo irtRadiator;
        unsigned npe_per_radiator = 0, nhits_per_radiator = 0;

        nhits_per_radiator = particle->GetRecoCherenkovPhotonCount(radiator);
        irtRadiator.setNhits(nhits_per_radiator);
        nhits_per_track += nhits_per_radiator;

        for (auto photon : particle->GetHistory(rhptr)->Photons())
          if (photon->WasDetected())
            npe_per_radiator++;

        printf("  Here-Q2: %4d\n", npe_per_radiator); //track, nhits_per_track);
        irtRadiator.setNpe(npe_per_radiator);
        npe_per_track += npe_per_radiator;

        irtRadiator.setAngle(1000 * particle->GetRecoCherenkovAverageTheta(radiator));

        out_irt_radiator_info->push_back(irtRadiator);
        irtParticle.addToRadiators(irtRadiator);
      } //for rhistory
      printf("Here-Q3: %4d %4d\n", npe_per_track, nhits_per_track);

      irtParticle.setPDG(particle->GetPDG());
      irtParticle.setNpe(npe_per_track);
      irtParticle.setNhits(nhits_per_track);

      out_irt_particles->push_back(irtParticle);
    } //for particle
  }
} // IrtInterface::process()
} // namespace eicrecon

#endif
