// Copyright 2023, Christopher Dilks
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
  m_analysis_algo.AlgorithmInit(
      { "Aerogel", "Gas", "Merged Aerogel+Gas"},
      m_log
      );
}

//-------------------------------------------
// ProcessSequential
//-------------------------------------------
void eicrecon::IrtCherenkovParticleID_processor::ProcessSequential(const std::shared_ptr<const JEvent>& event) {
  std::map<std::string, std::vector<const edm4eic::CherenkovParticleID*>> cherenkov_pids = { // FIXME: generalize
    { "Aerogel",            m_aerogel_pids() },
    { "Gas",                m_gas_pids()     },
    { "Merged Aerogel+Gas", m_merged_pids()  }
  };
  m_analysis_algo.AlgorithmProcess(m_mc_parts(), m_sim_hits(), cherenkov_pids);
}

//-------------------------------------------
// FinishWithGlobalRootLock
//-------------------------------------------
void eicrecon::IrtCherenkovParticleID_processor::FinishWithGlobalRootLock() {
  m_analysis_algo.AlgorithmFinish();
}
