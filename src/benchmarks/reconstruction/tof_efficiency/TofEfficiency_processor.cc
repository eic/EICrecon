#include "TofEfficiency_processor.h"
#include <services/rootfile/RootFile_service.h>

#include <Evaluator/DD4hepUnits.h>
#include <TVector3.h>

//-------------------------------------------
// InitWithGlobalRootLock
//-------------------------------------------
void TofEfficiency_processor::InitWithGlobalRootLock(){
    std::string plugin_name=("tof_efficiency");

    InitLogger(plugin_name);

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
}

//-------------------------------------------
// ProcessSequential
//-------------------------------------------
void TofEfficiency_processor::ProcessSequential(const std::shared_ptr<const JEvent>& event) {

    // List TOF Barrel hits from barrel
    logger()->trace("TOF barrel hits:");
    m_log->trace("   {:>10} {:>10} {:>10} {:>10}", "[x]", "[y]", "[z]", "[time]");
    for (auto hit: barrelHits()) {
        auto& pos = hit->getPosition();
        m_log->trace("   {:>10.2f} {:>10.2f} {:>10.2f} {:>10.4f}", pos.x, pos.y, pos.z, hit->getTime());
    }

    // List TOF endcap hits
    logger()->trace("TOF endcap hits:");
    m_log->trace("   {:>10} {:>10} {:>10} {:>10}", "[x]", "[y]", "[z]", "[time]");
    for (auto hit: endcapHits()) {
        auto& pos = hit->getPosition();
        m_log->trace("   {:>10.2f} {:>10.2f} {:>10.2f} {:>10.4f}", pos.x, pos.y, pos.z, hit->getTime());
    }

    // Now go through reconstructed tracks points
    logger()->trace("Going over tracks:");
    m_log->trace("   {:>10} {:>10} {:>10} {:>10}", "[x]", "[y]", "[z]", "[length]");
    for( auto track_segment : trackSegments() ){
        logger()->trace(" Track trajectory");

        for(auto point: track_segment->getPoints()) {
            auto &pos = point.position;
            m_log->trace("   {:>10.2f} {:>10.2f} {:>10.2f} {:>10.2f}", pos.x, pos.y, pos.z, point.pathlength);
        }
    }
}

//-------------------------------------------
// FinishWithGlobalRootLock
//-------------------------------------------
void TofEfficiency_processor::FinishWithGlobalRootLock() {

    // Do any final calculations here.

}

