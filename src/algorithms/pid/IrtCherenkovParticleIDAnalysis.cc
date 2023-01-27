// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "IrtCherenkovParticleIDAnalysis.h"

// AlgorithmInit
//---------------------------------------------------------------------------
void eicrecon::IrtCherenkovParticleIDAnalysis::AlgorithmInit(std::shared_ptr<spdlog::logger>& logger) {
  m_log = logger;
  for(auto& [id,rad_name] : Tools::GetRadiatorIDs())
    m_radiator_histos.insert({id,std::make_shared<RadiatorAnalysis>(TString(rad_name))});
}


// AlgorithmProcess
//---------------------------------------------------------------------------
void eicrecon::IrtCherenkovParticleIDAnalysis::AlgorithmProcess(std::vector<const edm4eic::CherenkovParticleID*> cherenkov_pids) {
  m_log->trace("{:=^70}"," call IrtCherenkovParticleIDAnalysis::AlgorithmProcess ");

  // loop over `CherenkovParticleID` objects
  for(const auto& pid : cherenkov_pids) {

    // get the histograms for this radiator
    std::shared_ptr<RadiatorAnalysis> radiator_histos;
    TString rad_name;
    try {
      radiator_histos = m_radiator_histos.at(pid->getRadiator());
      rad_name = radiator_histos->GetRadiatorName();
    }
    catch(const std::out_of_range& e) {
      m_log->error("Invalid radiator number {}", pid->getRadiator());
      continue;
    }
    m_log->trace("-> {} Radiator (ID={}):", rad_name, pid->getRadiator());

    // estimate the charged particle energy using the momentum of the first TrackPoint at this radiator's entrance
    auto charged_particle = pid->getChargedParticle();
    if(!charged_particle.isAvailable())   { m_log->warn("Charged particle not available in this radiator");      continue; }
    if(charged_particle.points_size()==0) { m_log->warn("Charged particle has no TrackPoints in this radiator"); continue; }
    auto charged_particle_momentum = edm4hep::utils::magnitude( charged_particle.getPoints(0).momentum );
    m_log->trace("  Charged Particle p = {} GeV at radiator entrance", charged_particle_momentum);
    m_log->trace("  If it is a pion, E = {} GeV", std::hypot(charged_particle_momentum, Tools::GetPDGMass(211)));

    // trace logging for IRT results
    m_log->trace("  Cherenkov Angle Estimate:");
    m_log->trace("    {:>16}:  {:<10}",     "NPE",          pid->getNpe());
    m_log->trace("    {:>16}:  {:<10.3}",   "<theta>",      pid->getTheta());
    m_log->trace("    {:>16}:  {:<10.3}",   "<rindex>",     pid->getRindex());
    m_log->trace("    {:>16}:  {:<10.3}",   "<wavelength>", pid->getWavelength());
    m_log->trace("  Mass Hypotheses:");
    m_log->trace("    {:>6}  {:>10}  {:>10}", "PDG", "Weight", "NPE");
    for(const auto& hyp : pid->getHypotheses())
      m_log->trace("    {:>6}  {:>10.8}  {:>10.8}", hyp.pdg, hyp.weight, hyp.npe);

    // Cherenkov angle estimate and NPE
    radiator_histos->m_npe_dist->Fill(pid->getNpe());
    radiator_histos->m_npe_vs_p->Fill(charged_particle_momentum,pid->getNpe());
    radiator_histos->m_theta_dist->Fill(pid->getTheta()*1e3); // [rad] -> [mrad]
    radiator_histos->m_theta_vs_p->Fill(charged_particle_momentum,pid->getTheta()*1e3); // [rad] -> [mrad]
    for(const auto& [theta,phi] : pid->getThetaPhiPhotons())
      radiator_histos->m_photon_theta_vs_phi->Fill(phi,theta*1e3); // [rad] -> [mrad]

    // fill MC dists
    radiator_histos->m_mc_wavelength->Fill(pid->getWavelength());
    radiator_histos->m_mc_rindex->Fill(pid->getRindex());

    // find the PDG hypothesis with the highest weight
    float max_weight     = -1000;
    int   pdg_max_weight = 0;
    for(const auto& hyp : pid->getHypotheses()) {
      if(hyp.weight > max_weight) {
        max_weight     = hyp.weight;
        pdg_max_weight = hyp.pdg;
      }
    }
    std::string pdg_max_weight_str = "UNKNOWN";
    if(pdg_max_weight!=0 && !std::isnan(pdg_max_weight))
      pdg_max_weight_str = std::to_string(pdg_max_weight);
    m_log->trace(" Highest weight is {} for PDG {} (string='{}')", max_weight, pdg_max_weight, pdg_max_weight_str);
    radiator_histos->m_highest_weight_dist->Fill(pdg_max_weight_str.c_str(), 1);
    radiator_histos->m_highest_weight_vs_p->Fill(charged_particle_momentum, pdg_max_weight_str.c_str(), 1);

  }
}


// AlgorithmFinish
//---------------------------------------------------------------------------
void eicrecon::IrtCherenkovParticleIDAnalysis::AlgorithmFinish() {
}
