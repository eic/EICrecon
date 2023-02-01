// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "IrtCherenkovParticleID_processor.h"

//-------------------------------------------
// InitWithGlobalRootLock
//-------------------------------------------
void eicrecon::IrtCherenkovParticleID_processor::InitWithGlobalRootLock() {
  auto app = GetApplication();

  // input tags
  auto plugin_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");

  // set logger
  InitLogger(plugin_name, "info");
  m_log->debug("Initializing plugin {}",plugin_name);

  // set ROOT TDirectory
  auto rootfile_svc = app->GetService<RootFile_service>();
  auto rootfile = rootfile_svc->GetHistFile();
  rootfile->mkdir("pid_irt")->cd();

  // initialize underlying algorithms
  m_analysis_algo.AlgorithmInit(m_log);
}

//-------------------------------------------
// ProcessSequential
//-------------------------------------------
void eicrecon::IrtCherenkovParticleID_processor::ProcessSequential(const std::shared_ptr<const JEvent>& event) {
  m_analysis_algo.AlgorithmProcess(m_sim_hits(),m_cherenkov_pids());
}

//-------------------------------------------
// FinishWithGlobalRootLock
//-------------------------------------------
void eicrecon::IrtCherenkovParticleID_processor::FinishWithGlobalRootLock() {
  m_analysis_algo.AlgorithmFinish();
}
