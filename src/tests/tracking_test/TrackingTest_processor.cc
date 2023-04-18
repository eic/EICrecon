
#include "TrackingTest_processor.h"
#include "algorithms/tracking/JugTrack/TrackingResultTrajectory.hpp"
#include "extensions/spdlog/SpdlogExtensions.h"

#include "services/io/podio/datamodel_glue.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <fmt/core.h>

#include <Math/LorentzVector.h>
#include <Math/GenVector/PxPyPzM4D.h>

#include <spdlog/spdlog.h>
#include <edm4hep/MCParticle.h>
#include <edm4eic/TrackParameters.h>
#include <edm4eic/ReconstructedParticle.h>

#include <algorithms/tracking/ParticlesFromTrackFitResult.h>
#include <algorithms/tracking/JugTrack/Track.hpp>
#include <services/rootfile/RootFile_service.h>

using namespace fmt;

//------------------
// OccupancyAnalysis (Constructor)
//------------------
TrackingTest_processor::TrackingTest_processor(JApplication *app) :
	JEventProcessor(app)
{
}

//------------------
// Init
//------------------
void TrackingTest_processor::Init()
{
    std::string plugin_name=("tracking_test");

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

    // Get log level from user parameter or default
    std::string log_level_str = "info";
    m_log = app->GetService<Log_service>()->logger(plugin_name);
    app->SetDefaultParameter(plugin_name + ":LogLevel", log_level_str, "LogLevel: trace, debug, info, warn, err, critical, off");
    m_log->set_level(eicrecon::ParseLogLevel(log_level_str));
    for(auto pair: app->GetJParameterManager()->GetAllParameters()) {
        m_log->info("{:<20} | {}", pair.first, pair.second->GetDescription());
    }
}


//------------------
// Process
//------------------
void TrackingTest_processor::Process(const std::shared_ptr<const JEvent>& event)
{
    m_log->debug("---- TrackingTest_processor {} ----", event->GetEventNumber());

    ProcessTrackingMatching(event);
}


//------------------
// Finish
//------------------
void TrackingTest_processor::Finish()
{
	fmt::print("OccupancyAnalysis::Finish() called\n");

}

void TrackingTest_processor::ProcessTrackingResults(const std::shared_ptr<const JEvent> &event) {
    auto reco_particles = event->GetCollection<edm4eic::ReconstructedParticle>("outputParticles");

    m_log->debug("Tracking reconstructed particles N={}: ", reco_particles->size());
    m_log->debug("   {:<5} {:>8} {:>8} {:>8} {:>8} {:>8}","[i]", "[px]", "[py]", "[pz]", "[P]", "[P*3]");

    for(size_t i=0; i < reco_particles->size(); i++) {
        auto particle = (*reco_particles)[i];

        double px = particle.getMomentum().x;
        double py = particle.getMomentum().y;
        double pz = particle.getMomentum().z;
        // ROOT::Math::PxPyPzM4D p4v(px, py, pz, particle.getMass());
        ROOT::Math::Cartesian3D p(px, py, pz);
        m_log->debug("   {:<5} {:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f}", i,  px, py, pz, p.R(), p.R()*3);
    }

    auto mc_particles = event->Get<edm4hep::MCParticle>("MCParticles");

    auto particles = event->GetSingle<edm4eic::ReconstructedParticle>("ReconstructedParticles");
    auto track_params = event->GetSingle<edm4eic::TrackParameters>("outputTrackParameters");

    m_log->debug("MC particles N={}: ", mc_particles.size());
    m_log->debug("   {:<5} {:<6} {:<7} {:>8} {:>8} {:>8} {:>8}","[i]", "status", "[PDG]",  "[px]", "[py]", "[pz]", "[P]");
    for(size_t i=0; i < mc_particles.size(); i++) {

        auto particle=mc_particles[i];

        if(particle->getGeneratorStatus() != 1) continue;
//
        double px = particle->getMomentum().x;
        double py = particle->getMomentum().y;
        double pz = particle->getMomentum().z;
        ROOT::Math::PxPyPzM4D p4v(px, py, pz, particle->getMass());
        ROOT::Math::Cartesian3D p(px, py, pz);
        if(p.R()<1) continue;

        m_log->debug("   {:<5} {:<6} {:<7} {:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f}", i, particle->getGeneratorStatus(), particle->getPDG(),  px, py, pz, p.R());
    }

}

void TrackingTest_processor::ProcessTrackingMatching(const std::shared_ptr<const JEvent> &event) {
    m_log->debug("Associations [simId] [recID] [simE] [recE] [simPDG] [recPDG]");
    // auto prt_with_assoc = event->GetSingle<edm4hep::ReconstructedParticle>("ChargedParticlesWithAssociations");

    auto particles = event->GetCollection<edm4eic::ReconstructedParticle>("ReconstructedChargedParticles");
    auto associations = event->GetCollection<edm4eic::MCRecoParticleAssociation>("ReconstructedChargedParticlesAssociations");



    for(auto assoc: *associations) {
        auto sim = assoc.getSim();
        auto rec = assoc.getRec();

        m_log->debug("  {:<6} {:<6} {:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f}", assoc.getSimID(), assoc.getRecID(), sim.getPDG(), rec.getPDG());
    }

//    m_log->debug("Particles [objID] [PDG] [simE] [recE] [simPDG] [recPDG]");
//    for(auto part: prt_with_assoc->particles()) {
//
//        // auto sim = assoc->getSim();
//        // auto rec = assoc->getRec();
//
//        m_log->debug("  {:<6} {:<6}  {:>8.2f} {:>8.2f}", part->getObjectID().index, part->getPDG(), part->getCharge(), part->getEnergy());
//
//    }
//
//
//    m_log->debug("ReconstructedChargedParticles [objID] [PDG] [charge] [energy]");
//    auto reco_charged_particles = event->Get<edm4eic::ReconstructedParticle>("ReconstructedChargedParticles");
//    for(auto part: reco_charged_particles) {
//        m_log->debug("  {:<6} {:<6}  {:>8.2f} {:>8.2f}", part->getObjectID().index, part->getPDG(), part->getCharge(), part->getEnergy());
//    }

}

void TrackingTest_processor::ProcessGloablMatching(const std::shared_ptr<const JEvent> &event) {

    m_log->debug("ReconstructedParticles (FINAL) [objID] [PDG] [charge] [energy]");
    auto final_reco_particles = event->Get<edm4eic::ReconstructedParticle>("ReconstructedParticlesWithAssoc");
    for(auto part: final_reco_particles) {
        m_log->debug("  {:<6} {:<6}  {:>8.2f} {:>8.2f}", part->getObjectID().index, part->getPDG(), part->getCharge(), part->getEnergy());
    }

    auto final_generated_particles = event->GetSingle<edm4eic::ReconstructedParticle>("GeneratedParticles");
}
