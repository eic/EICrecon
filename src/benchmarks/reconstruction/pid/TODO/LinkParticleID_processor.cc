// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "LinkParticleID_processor.h"

//-------------------------------------------
// InitWithGlobalRootLock
//-------------------------------------------
void eicrecon::LinkParticleID_processor::InitWithGlobalRootLock() {
  auto app = GetApplication();

  // input tags
  auto plugin_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");

  // set logger
  InitLogger(plugin_name, "info");
  m_log->debug("Initializing plugin {}",plugin_name);

  // set ROOT TDirectory
  auto rootfile_svc = app->GetService<RootFile_service>();
  auto rootfile = rootfile_svc->GetHistFile();
  m_rootdir = rootfile->mkdir("pid_link");
  m_rootdir->cd();

  // initialize underlying algorithms
  m_analysis_algo.AlgorithmInit(m_log);
}

//-------------------------------------------
// ProcessSequential
//-------------------------------------------
void eicrecon::LinkParticleID_processor::ProcessSequential(const std::shared_ptr<const JEvent>& event) {
  m_analysis_algo.AlgorithmProcess(m_assocs());
}

//-------------------------------------------
// FinishWithGlobalRootLock
//-------------------------------------------
void eicrecon::LinkParticleID_processor::FinishWithGlobalRootLock() {
  m_rootdir->cd();
  m_analysis_algo.AlgorithmFinish();
}
