// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "Digitizer_processor.h"

//-------------------------------------------
// InitWithGlobalRootLock
//-------------------------------------------
void eicrecon::Digitizer_processor::InitWithGlobalRootLock() {
  auto app = GetApplication();

  // input tags
  auto plugin_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");

  // set logger
  InitLogger(plugin_name, "info");
  m_log->debug("Initializing plugin {}",plugin_name);

  // set ROOT TDirectory
  auto rootfile_svc = app->GetService<RootFile_service>();
  auto rootfile = rootfile_svc->GetHistFile();
  rootfile->mkdir("pid_digi")->cd();

  // initialize underlying algorithms
  m_analysis_algo.AlgorithmInit(m_log);
}

//-------------------------------------------
// ProcessSequential
//-------------------------------------------
void eicrecon::Digitizer_processor::ProcessSequential(const std::shared_ptr<const JEvent>& event) {
  m_analysis_algo.AlgorithmProcess(m_digi_hits());
}

//-------------------------------------------
// FinishWithGlobalRootLock
//-------------------------------------------
void eicrecon::Digitizer_processor::FinishWithGlobalRootLock() {
  m_analysis_algo.AlgorithmFinish();
}
