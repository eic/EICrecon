// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "BenchmarksPID_processor.h"

//-------------------------------------------
// InitWithGlobalRootLock
//-------------------------------------------
void eicrecon::BenchmarksPID_processor::InitWithGlobalRootLock() {
  auto app = GetApplication();

  // input tags
  auto plugin_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");

  // set logger
  InitLogger(plugin_name, "info");
  m_log->debug("Initializing plugin {}",plugin_name);

  // set ROOT TDirectory
  auto rootfile_svc = app->GetService<RootFile_service>();
  auto rootfile = rootfile_svc->GetHistFile();
  rootfile->mkdir("pid")->cd();

  // initialize underlying algorithm
  m_analysis_algo.AlgorithmInit(m_log);
}

//-------------------------------------------
// ProcessSequential
//-------------------------------------------
void eicrecon::BenchmarksPID_processor::ProcessSequential(const std::shared_ptr<const JEvent>& event) {
  m_analysis_algo.AlgorithmProcess(m_cherenkov_pids());
}

//-------------------------------------------
// FinishWithGlobalRootLock
//-------------------------------------------
void eicrecon::BenchmarksPID_processor::FinishWithGlobalRootLock() {
  m_analysis_algo.AlgorithmFinish();
}
