#include "TofEfficiency_processor.h"
#include <services/rootfile/RootFile_service.h>

#include <Evaluator/DD4hepUnits.h>
#include <TVector3.h>

//-------------------------------------------
// InitWithGlobalRootLock
//-------------------------------------------
void TofEfficiency_processor::InitWithGlobalRootLock(){
    std::string plugin_name=("tof_efficiency");

    InitLogger(plugin_name, "info");

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

    //

}

//-------------------------------------------
// ProcessSequential
//-------------------------------------------
void TofEfficiency_processor::ProcessSequential(const std::shared_ptr<const JEvent>& event) {

    // Fill histograms here
    for( auto track_segment : trackSegments() ){
    }
}

//-------------------------------------------
// FinishWithGlobalRootLock
//-------------------------------------------
void TofEfficiency_processor::FinishWithGlobalRootLock() {

    // Do any final calculations here.

}

