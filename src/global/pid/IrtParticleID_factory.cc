// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

/* - input: photoelectrons, tracking info
 * - output: Cherenkov PID hypothesis and emission angle
 * - prepares for and calls the IRT (standalone) algorithm
 */

#include "IrtParticleID_factory.h"

//-----------------------------------------------------------------------------
void eicrecon::IrtParticleID_factory::Init() {
  auto app = GetApplication();

  // default params
  m_detector_name = "DRICH"; // FIXME: respect https://github.com/eic/EICrecon/pull/242
  auto tag = "RICH:"+GetTag();
  app->SetDefaultParameter(tag+":which_rich", m_detector_name, "Indicate which RICH to use");

  // services
  m_richGeoSvc = app->template GetService<RichGeo_service>();
  m_irtDetectorCollection = m_richGeoSvc->GetIrtGeo(m_detector_name)->GetIrtDetectorCollection();
  m_log = app->GetService<Log_service>()->logger(GetTag()); // FIXME: use SpdlogMixin

  // set log level
  m_log = japp->GetService<Log_service>()->logger(GetTag());

  m_log->info("\n\nUSING RICH: {}\n\n",m_detector_name);

}

//-----------------------------------------------------------------------------
void eicrecon::IrtParticleID_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
}

//-----------------------------------------------------------------------------
void eicrecon::IrtParticleID_factory::Process(const std::shared_ptr<const JEvent> &event) {

  // inputs
  // FIXME: will be changed to photoelectrons; until then, just use all the photon hits
  auto photoelectrons = event->Get<edm4hep::SimTrackerHit>(m_detector_name+"Hits");

  // loop over photoelectrons
  // FIXME: at the moment, we do nothing; the current version of this factory is only meant to test the `rich` service
  std::vector<edm4hep::ParticleID*> output_pid;
  // for( const auto& photoelectron : photoelectrons ) {
  //   auto pid = new edm4hep::ParticleID(
  //       ...
  //       );
  //   output_pid.push_back(pid);
  // }

  // outputs
  Set(output_pid);
}
