// Copyright 2022, Christopher Dilks, adapted from Alexander Kiselev's Juggler implementation `IRTAlgorithm`
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "IrtParticleID.h"

// AlgorithmInit
//---------------------------------------------------------------------------
void eicrecon::IrtParticleID::AlgorithmInit(
    CherenkovDetectorCollection     *irt_det_coll,
    std::shared_ptr<spdlog::logger> &logger
    )
{
  // members
  m_irt_det_coll = irt_det_coll;
  m_log          = logger;
  m_init_failed  = false;

  // print the configuration parameters
  m_cfg.Print(m_log, spdlog::level::debug);

  // extract the the relevant `CherenkovDetector`, set to `m_irt_det`
  auto &detectors = m_irt_det_coll->GetDetectors();
  if(detectors.size() == 0) {
    m_log->error("No CherenkovDetectors found in input collection `irt_det_coll`");
    m_init_failed = true;
  }
  if(detectors.size() > 1)
    m_log->warn("IrtParticleID currently only supports 1 CherenkovDetector at a time; taking the first");
  auto this_detector = *detectors.begin();
  m_det_name = this_detector.first;
  m_irt_det = this_detector.second;
  m_log->debug("Initializing IrtParticleID algorithm for CherenkovDetector '{}'", m_det_name);

  // readout decoding
  m_cell_mask = m_irt_det->GetReadoutCellMask();
  m_log->debug("readout cellMask = {:#X}", m_cell_mask);

  // rebin refractive index tables to have `m_cfg.numRIndexBins` bins
  m_log->trace("Rebinning refractive index tables to have {} bins",m_cfg.numRIndexBins);
  for(auto [rad_name,irt_rad] : m_irt_det->Radiators()) {
    auto ri_lookup_table_orig = irt_rad->m_ri_lookup_table;
    irt_rad->m_ri_lookup_table.clear();
    irt_rad->m_ri_lookup_table = Tools::ApplyFineBinning( ri_lookup_table_orig, m_cfg.numRIndexBins );
    // m_log->trace("- {}", rad_name);
    // for(auto [energy,rindex] : irt_rad->m_ri_lookup_table) m_log->trace("  {:>5} eV   {:<}", energy, rindex);
  }

  // check radiators' configuration, and pass it to `m_irt_det`'s radiators
  for(auto [rad_name,irt_rad] : m_irt_det->Radiators()) {
    // find `cfg_rad`, the associated `IrtParticleIDConfig` radiator
    auto cfg_rad_it = m_cfg.radiators.find(std::string(rad_name));
    if(cfg_rad_it != m_cfg.radiators.end()) {
      auto cfg_rad = cfg_rad_it->second;
      // pass `cfg_rad` params to `irt_rad`, the IRT radiator
      irt_rad->m_AverageRefractiveIndex = cfg_rad.referenceRIndex;
      irt_rad->SetReferenceRefractiveIndex(cfg_rad.referenceRIndex);
      if(cfg_rad.attenuation>0)
        irt_rad->SetReferenceAttenuationLength(cfg_rad.attenuation / dd4hep::mm);
      if(cfg_rad.smearing>0) {
        if(cfg_rad.smearingMode=="uniform")
          irt_rad->SetUniformSmearing(cfg_rad.smearing / dd4hep::radian);
        else if(cfg_rad.smearingMode=="gaussian")
          irt_rad->SetGaussianSmearing(cfg_rad.smearing / dd4hep::radian);
        else
          m_log->error("Unknown smearing mode '{}' for {} radiator",cfg_rad.smearingMode,rad_name);
      }
      irt_rad->SetTrajectoryBinCount(cfg_rad.zbins);
    } else {
      if(rad_name!="Filter")
        m_log->error("Cannot find radiator '{}' in IrtParticleIDConfig instance",rad_name);
    }
  }

}


// AlgorithmChangeRun
//---------------------------------------------------------------------------
void eicrecon::IrtParticleID::AlgorithmChangeRun() {
}


// AlgorithmProcess
//---------------------------------------------------------------------------
std::vector<edm4hep::ParticleID*> eicrecon::IrtParticleID::AlgorithmProcess(
    std::vector<const edm4eic::RawPMTHit*>& in_raw_hits,
    std::map<std::string,std::vector<const edm4eic::TrackSegment*>>& in_charged_particles
    )
{
  // logging
  m_log->trace("{:=^70}"," call IrtParticleID::AlgorithmProcess ");
  m_log->trace("number of raw sensor hits: {}", in_raw_hits.size());

  // start output collections
  std::vector<edm4hep::ParticleID*> particle_id;
  if(m_init_failed) return particle_id;

  // check `in_charged_particles`: each radiator should have the same number of TrackSegments
  long num_charged_particles = -1;
  for(const auto& [rad_name,charged_particle_list] : in_charged_particles) {
    if(num_charged_particles<0) {
      num_charged_particles = charged_particle_list.size();
      m_log->trace("number of reconstructed charged particles: {}", charged_particle_list.size());
    }
    else if(num_charged_particles != charged_particle_list.size()) {
      m_log->error("radiators have differing numbers of TrackSegments");
      return particle_id;
    }
  }


  // loop over raw hits ***************************************************
  // - make `irt_photons`: a list of `OpticalPhoton`s for the IRT algorithm
  std::vector<std::unique_ptr<OpticalPhoton>> irt_photons;
  m_log->trace("{:#<70}","### SENSOR HITS ");
  for(const auto& raw_hit : in_raw_hits) {
    m_log->trace("{:-<70}","--- pixel hit ");

    // get sensor and pixel info
    auto cell_id       = raw_hit->getCellID();
    uint64_t sensor_id = cell_id & m_cell_mask;
    TVector3 pixel_pos = (1/dd4hep::mm) * m_irt_det->m_ReadoutIDToPosition(cell_id);
    m_log->trace("cell_id={:#X}  copy={}  pixel_pos=( {:>10.2f} {:>10.2f} {:>10.2f} )",
        cell_id, sensor_id, pixel_pos.x(), pixel_pos.y(), pixel_pos.z());
    // FIXME: `pixel_pos` is slightly different from juggler (but who is right?)

    // start new IRT photon
    auto irt_sensor = m_irt_det->m_PhotonDetectors[0]; // FIXME: assumes one sensor type
    auto irt_photon = std::make_unique<OpticalPhoton>();
    irt_photon->SetVolumeCopy(sensor_id);
    irt_photon->SetDetectionPosition(pixel_pos);
    irt_photon->SetPhotonDetector(irt_sensor);
    irt_photon->SetDetected(true);

    // FIXME these require knowlege of the truth! do we need anything here?
    /*
    irt_photon->SetVertexPosition(vtx);
    irt_photon->SetVertexMomentum(TVector3(p.x, p.y, p.z));
    // Retrieve a refractive index estimate; it is not exactly the one, which 
    // was used in GEANT, but should be very close;
    double ri;
    auto eVenergy = // TODO: photon energy in eV
    auto radiator = // TODO
    Tools::GetFinelyBinnedTableEntry(radiator->m_ri_lookup_table, eVenergy, &ri);
    irt_photon->SetVertexRefractiveIndex(ri);
    */

    // fill `irt_photons`
    irt_photons.push_back(std::move(irt_photon));
  }

  /*

  // loop over charged particles ********************************************
  // - make `irt_particles`: a list of `ChargedParticle`s for the IRT algorithm
  std::vector<std::unique_ptr<ChargedParticle>> irt_particles;
  for(long i=0; i<num_charged_particles; i++) {

    // start an `irt_particle`
    auto irt_particle = std::make_unique<ChargedParticle>();
    m_log->trace("{:-<70}",fmt::format("--- charged particle #{} ",i));

    // loop over IRT radiators, starting a `RadiatorHistory` for each
    for(auto [rad_name,irt_rad] : m_irt_det->Radiators())
      irt_particle->StartRadiatorHistory({ irt_rad, new RadiatorHistory() });

    // loop over radiators in `in_charged_particles`
    for(const auto& [rad_name,charged_particle_list] : in_charged_particles) {

      // find the IRT radiator
      auto irt_rad = m_irt_det->GetRadiator(rad_name.c_str());
      if(!irt_rad) {
        else m_log->error("WARNING: cannot find IRT radiator '{}'",rad_name);
        continue;
      }

      // loop over `irt_photons`
      // - add each `irt_photon` to the radiator history
      auto irt_rad_history = irt_particle->FindRadiatorHistory(irt_rad);
      for(auto irt_photon : irt_photons) {
        irt_rad_history->AddOpticalPhoton(irt_photon);
      }

      // loop over `TrackPoint`s of this `charged_particle`
      // - add each to the radiator history and radiator
      irt_rad->ResetLocations()
      m_log->trace("TrackPoints in '{}' radiator:",rad_name);
      auto charged_particle = charged_particle_list.at(i);
      for(const auto& point : charged_particle->getPoints()) {
        TVector3 position = Tools::PodioVector3_to_TVector3(point.position);
        TVector3 momentum = Tools::PodioVector3_to_TVector3(point.momentum);
        // irt_rad_history->AddStep( new ChargedParticleStep(position,momentum) ); // FIXME: not needed, if we can just use AddLocation?
        irt_rad->AddLocation(position,momentum);
        m_log->trace(" point: x=( {:>10.2f} {:>10.2f} {:>10.2f} )", position.x(), position.y(), position.z());
        m_log->trace("        p=( {:>10.2f} {:>10.2f} {:>10.2f} )", momentum.x(), momentum.y(), momentum.z());
      }

      //
      //
      // -> run IRT code for this `irt_particle`
      //
      //
      // CherenkovPID irt_pid;
      //  int pdg_table[] = {-11, 211, 321, 2212};
      //   for(unsigned ip=0; ip<sizeof(pdg_table)/sizeof(pdg_table[0]); ip++) {
      //     const auto &service = m_pidSvc->particle(pdg_table[ip]);
      //     irt_pid.AddMassHypothesis(service.mass);
      //   }
      // }

    // fill `irt_particles`
    irt_particles.push_back(std::move(irt_particle));
  }
  */

  return particle_id;
}
