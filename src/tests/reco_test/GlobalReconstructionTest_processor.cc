
#include "GlobalReconstructionTest_processor.h"
#include "extensions/spdlog/SpdlogExtensions.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <fmt/core.h>

#include <TDirectory.h>
#include <TCanvas.h>
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <Math/LorentzVector.h>
#include <Math/GenVector/PxPyPzM4D.h>

#include <spdlog/spdlog.h>

#include <edm4hep/SimCalorimeterHit.h>
#include <edm4hep/MCParticle.h>
#include <edm4eic/TrackerHit.h>
#include <edm4eic/TrackParameters.h>
#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/MCRecoParticleAssociation.h>

#include "services/log/Log_service.h"
#include "services/rootfile/RootFile_service.h"

using namespace fmt;

//------------------
// OccupancyAnalysis (Constructor)
//------------------
GlobalReconstructionTest_processor::GlobalReconstructionTest_processor(JApplication *app) :
        JEventProcessor(app)
{
}

//------------------
// Init
//------------------
void GlobalReconstructionTest_processor::Init()
{
    std::string plugin_name=("reco_test");

    // Get JANA application
    auto *app = GetApplication();

    // Ask service locator a file to write histograms to
    auto root_file_service = app->GetService<RootFile_service>();

    // Get TDirectory for histograms root file
    auto globalRootLock = app->GetService<JGlobalRootLock>();
    globalRootLock->acquire_write_lock();
    auto *file = root_file_service->GetHistFile();
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
void GlobalReconstructionTest_processor::Process(const std::shared_ptr<const JEvent>& event)
{
    using namespace ROOT;

    m_log->debug("----------- GlobalReconstructionTest_processor {}-----------", event->GetEventNumber());

    /** RECONSTRUCTED PARTICLES **/
    auto reco_particles = event->Get<edm4eic::ReconstructedParticle>("ReconstructedParticles");
    printRecoParticles(reco_particles, std::string("ReconstructedParticles"));

    /** GENERATED PARTICLES **/
    auto gen_particles = event->Get<edm4eic::ReconstructedParticle>("GeneratedParticles");
    printRecoParticles(gen_particles, std::string("GeneratedParticles"));

    /** MC PARTICLES **/
    auto mc_particles = event->Get<edm4hep::MCParticle>("MCParticles");
    m_log->debug("MC particles N={}: ", mc_particles.size());
    m_log->debug("   {:<5} {:<6} {:<7} {:>8} {:>8} {:>8} {:>8}","[i]", "status", "[PDG]",  "[px]", "[py]", "[pz]", "[P]");
    for(size_t i=0; i < mc_particles.size(); i++) {

        const auto *particle=mc_particles[i];

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


//------------------
// Finish
//------------------
void GlobalReconstructionTest_processor::Finish()
{
        fmt::print("GlobalReconstructionTest_processor::Finish() called\n");

}

void GlobalReconstructionTest_processor::printRecoParticles(std::vector<const edm4eic::ReconstructedParticle*> reco_particles, const std::string &title) {
    m_log->debug("{} N={}: ", title, reco_particles.size());
    m_log->debug("   {:<5} {:>8} {:>8} {:>8} {:>8} {:>8}","[i]", "[px]", "[py]", "[pz]", "[P]", "[P*3]");


    for(size_t i=0; i < reco_particles.size(); i++) {
        const auto *particle = reco_particles[i];

        double px = particle->getMomentum().x;
        double py = particle->getMomentum().y;
        double pz = particle->getMomentum().z;
        // ROOT::Math::PxPyPzM4D p4v(px, py, pz, particle.getMass());
        ROOT::Math::Cartesian3D p(px, py, pz);
        m_log->debug("   {:<5} {:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f} {:>8.2f}", i,  px, py, pz, p.R(), p.R()*3);
    }
}
