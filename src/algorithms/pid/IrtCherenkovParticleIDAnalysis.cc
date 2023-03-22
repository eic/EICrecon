// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "IrtCherenkovParticleIDAnalysis.h"


// RadiatorAnalysis constructor: defines histograms for a radiator
//---------------------------------------------------------------------------
eicrecon::RadiatorAnalysis::RadiatorAnalysis(TString rad_name) : m_rad_name(rad_name) {

  // distributions
  m_npe_dist = new TH1D(
      "npe_dist_"+m_rad_name,
      "Overall NPE for "+m_rad_name+";NPE",
      npe_bins, 0, npe_max
      );
  m_theta_dist = new TH1D(
      "theta_dist_"+m_rad_name,
      "Estimated Cherenkov Angle for "+m_rad_name+";#theta [mrad]",
      theta_bins, 0, theta_max
      );
  m_thetaResid_dist = new TH1D(
      "thetaResid_dist_"+m_rad_name,
      "Estimated Cherenkov Angle Residual for "+m_rad_name+";#Delta#theta [mrad]",
      theta_bins, -thetaResid_max, thetaResid_max
      );
  m_photonTheta_vs_photonPhi = new TH2D(
      "photonTheta_vs_photonPhi_"+m_rad_name,
      "Estimated Photon #theta vs #phi for "+m_rad_name+";#phi [rad];#theta [mrad]",
      phi_bins, -TMath::Pi(), TMath::Pi(),
      theta_bins, 0, theta_max
      );

  // truth
  m_mcWavelength_dist = new TH1D(
      "mcWavelength_"+m_rad_name,
      "MC Photon Wavelength for "+m_rad_name+";#lambda [nm]",
      n_bins, 0, 1000
      );
  m_mcRindex_dist = new TH1D(
      "mcRindex_"+m_rad_name,
      "MC Refractive Index for "+m_rad_name+";n",
      10*n_bins, 0.99, 1.03
      );

  // PID
  m_highestWeight_dist = new TH1D(
      "highestWeight_dist_"+m_rad_name,
      "Highest PDG Weight for "+m_rad_name+";PDG",
      pdg_bins(), 0, pdg_bins()
      );

  // momentum scans
  m_npe_vs_p = new TH2D(
      "npe_vs_p_"+m_rad_name,
      "Overall NPE vs. Particle Momentum for "+m_rad_name+";p [GeV];NPE",
      momentum_bins, 0, momentum_max,
      npe_bins, 0, npe_max
      );
  m_theta_vs_p = new TH2D(
      "theta_vs_p_"+m_rad_name,
      "Estimated Cherenkov Angle vs. Particle Momentum for "+m_rad_name+";p [GeV];#theta [mrad]",
      momentum_bins, 0, momentum_max,
      theta_bins, 0, theta_max
      );
  m_thetaResid_vs_p = new TH2D(
      "thetaResid_vs_p_"+m_rad_name,
      "Estimated Cherenkov Angle Residual vs. Particle Momentum for "+m_rad_name+";p [GeV];#Delta#theta [mrad]",
      momentum_bins, 0, momentum_max,
      theta_bins, -thetaResid_max, thetaResid_max
      );
  m_highestWeight_vs_p = new TH2D(
      "highestWeight_vs_p_"+m_rad_name,
      "Highest PDG Weight vs. Particle Momentum for "+m_rad_name+";p [GeV]",
      momentum_bins, 0, momentum_max,
      pdg_bins(), 0, pdg_bins()
      );
}


// AlgorithmInit
//---------------------------------------------------------------------------
void eicrecon::IrtCherenkovParticleIDAnalysis::AlgorithmInit(std::shared_ptr<spdlog::logger>& logger) {
  m_log = logger;

  // initialize histograms for each radiator
  for(auto& [id,rad_name] : Tools::GetRadiatorIDs())
    m_radiator_histos.insert({id,std::make_shared<RadiatorAnalysis>(TString(rad_name))});

  // initialize common histograms
  m_nphot_vs_p = new TH2D("nphot_vs_p", "N_{photons} vs. Thrown Momentum;p [GeV];N_{photons}",
      RadiatorAnalysis::momentum_bins, 0, RadiatorAnalysis::momentum_max,
      RadiatorAnalysis::nphot_max,     0, RadiatorAnalysis::nphot_max
      );
  m_nphot_vs_p__transient = new TH1D("nphot_vs_p__transient", "",
      m_nphot_vs_p->GetNbinsX(),
      m_nphot_vs_p->GetXaxis()->GetXmin(),
      m_nphot_vs_p->GetXaxis()->GetXmax()
      );
}


// AlgorithmProcess
//---------------------------------------------------------------------------
void eicrecon::IrtCherenkovParticleIDAnalysis::AlgorithmProcess(
    std::vector<const edm4hep::MCParticle*>          mc_parts,
    std::vector<const edm4hep::SimTrackerHit*>       sim_hits,
    std::vector<const edm4eic::CherenkovParticleID*> cherenkov_pids
    )
{
  m_log->trace("{:=^70}"," call IrtCherenkovParticleIDAnalysis::AlgorithmProcess ");

  // get MC truth info
  int   n_thrown = 0;
  float thrown_momentum = 0.0;
  int   thrown_pdg = 0;
  for(const auto& part : mc_parts) {
    if(part->getGeneratorStatus()==1) {
      thrown_momentum = edm4hep::utils::p(*part);
      thrown_pdg      = part->getPDG();
      n_thrown++;
    }
  }
  if(n_thrown == 0) m_log->warn("no thrown particle found");
  if(n_thrown >  1) m_log->warn("this benchmark does not yet support multi-track events (n_thrown={})\n",n_thrown);

  // get the number of photons vs. thrown momentum for this event
  // - fill 1D thrown_momentum histogram `m_nphot_vs_p__transient` for each (pre-digitized) sensor hit
  for(const auto& hit : sim_hits) {
    // FIXME: get `thrown_momentum` from multi-track events; but in this attempt
    // below the parent momentum is not consistent; instead for now we use
    // `thrown_momentum` from truth
    /*
    auto photon = hit->getMCParticle();
    if(photon.parents_size()>0) thrown_momentum = edm4hep::utils::p(photon.getParents(0));
    */
    m_nphot_vs_p__transient->Fill(thrown_momentum);
  }
  // - use `m_nphot_vs_p__transient` results to fill 2D hist `m_nphot_vs_p` for this event
  for(int b=1; b<=m_nphot_vs_p__transient->GetNbinsX(); b++) {
    auto nphot = m_nphot_vs_p__transient->GetBinContent(b);
    if(nphot>0) {
      auto momentum = m_nphot_vs_p__transient->GetBinCenter(b);
      m_nphot_vs_p->Fill(momentum,nphot);
    }
  }
  // - clear `m_nphot_vs_p__transient` to be ready for the next event
  m_nphot_vs_p__transient->Reset();

  // loop over `CherenkovParticleID` objects
  for(const auto& pid : cherenkov_pids) {

    // skip if NPE==0
    if(pid->getNpe() == 0) {
      m_log->warn("Event found with NPE=0");
      continue;
    }

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

    // estimate the charged particle momentum using the momentum of the first TrackPoint at this radiator's entrance
    /*
    auto charged_particle = pid->getChargedParticle();
    if(!charged_particle.isAvailable())   { m_log->warn("Charged particle not available in this radiator");      continue; }
    if(charged_particle.points_size()==0) { m_log->warn("Charged particle has no TrackPoints in this radiator"); continue; }
    auto charged_particle_momentum = edm4hep::utils::magnitude( charged_particle.getPoints(0).momentum );
    m_log->trace("  Charged Particle p = {} GeV at radiator entrance", charged_particle_momentum);
    m_log->trace("  If it is a pion, E = {} GeV", std::hypot(charged_particle_momentum, Tools::GetPDGMass(211)));
    */
    // alternatively: use `thrown_momentum` (FIXME: will not work for multi-track events)
    auto charged_particle_momentum = thrown_momentum;
    auto charged_particle_pdg      = thrown_pdg;
    auto charged_particle_mass     = Tools::GetPDGMass(charged_particle_pdg);
    m_log->trace("  Charged Particle PDG={}, mass={} GeV, p={} GeV from truth",
        charged_particle_pdg, charged_particle_mass, charged_particle_momentum);

    // get average Cherenkov angle: `theta_rec`
    double theta_rec = 0.0;
    for(const auto& [theta,phi] : pid->getThetaPhiPhotons())
      theta_rec += theta;
    theta_rec /= pid->getNpe();

    // calculate expected Cherenkov angle `theta_exp` and residual `theta_resid`,
    // using refractive index from MC truth
    auto mc_rindex = pid->getRefractiveIndex(); // average refractive index for photons used in this `pid`
    auto theta_exp = TMath::ACos(
        TMath::Hypot( charged_particle_momentum, charged_particle_mass ) /
        ( mc_rindex * charged_particle_momentum )
        );
    auto theta_resid = theta_rec - theta_exp;

    // trace logging for IRT results
    m_log->trace("  Cherenkov Angle Estimate:");
    m_log->trace("    {:>16}:  {:<10}",        "NPE",      pid->getNpe());
    m_log->trace("    {:>16}:  {:<10.3} mrad", "<theta>",  theta_rec*1e3); // [rad] -> [mrad]
    m_log->trace("    {:>16}:  {:<10.3}",      "<rindex>", pid->getRefractiveIndex());
    m_log->trace("    {:>16}:  {:<10.3} eV",   "<energy>", pid->getPhotonEnergy()*1e9); // [GeV] -> [eV]
    m_log->trace("  Mass Hypotheses:");
    m_log->trace("    {:>6}  {:>10}  {:>10}", "PDG", "Weight", "NPE");
    for(const auto& hyp : pid->getHypotheses())
      m_log->trace("    {:>6}  {:>10.8}  {:>10.8}", hyp.PDG, hyp.weight, hyp.npe);

    // fill PID histograms
    radiator_histos->m_npe_dist->Fill(pid->getNpe());
    radiator_histos->m_npe_vs_p->Fill(charged_particle_momentum,pid->getNpe());
    radiator_histos->m_theta_dist->Fill(theta_rec*1e3); // [rad] -> [mrad]
    radiator_histos->m_theta_vs_p->Fill(charged_particle_momentum,theta_rec*1e3); // [rad] -> [mrad]
    radiator_histos->m_thetaResid_dist->Fill(theta_resid*1e3); // [rad] -> [mrad]
    radiator_histos->m_thetaResid_vs_p->Fill(charged_particle_momentum,theta_resid*1e3); // [rad] -> [mrad]
    for(const auto& [theta,phi] : pid->getThetaPhiPhotons())
      radiator_histos->m_photonTheta_vs_photonPhi->Fill(phi,theta*1e3); // [rad] -> [mrad]
    radiator_histos->m_mcWavelength_dist->Fill( HC / pid->getPhotonEnergy() ); // energy [GeV] -> wavelength [nm]
    radiator_histos->m_mcRindex_dist->Fill(mc_rindex);

    // find the PDG hypothesis with the highest weight
    float max_weight     = -1000;
    int   pdg_max_weight = 0;
    for(const auto& hyp : pid->getHypotheses()) {
      if(hyp.weight > max_weight) {
        max_weight     = hyp.weight;
        pdg_max_weight = hyp.PDG;
      }
    }
    std::string pdg_max_weight_str = "UNKNOWN";
    if(pdg_max_weight!=0 && !std::isnan(pdg_max_weight))
      pdg_max_weight_str = std::to_string(pdg_max_weight);
    m_log->trace(" Highest weight is {} for PDG {} (string='{}')", max_weight, pdg_max_weight, pdg_max_weight_str);
    radiator_histos->m_highestWeight_dist->Fill(pdg_max_weight_str.c_str(), 1);
    radiator_histos->m_highestWeight_vs_p->Fill(charged_particle_momentum, pdg_max_weight_str.c_str(), 1);

  }
}


// AlgorithmFinish
//---------------------------------------------------------------------------
void eicrecon::IrtCherenkovParticleIDAnalysis::AlgorithmFinish() {
  // delete transient histograms, so they don't get written
  delete m_nphot_vs_p__transient;
}
