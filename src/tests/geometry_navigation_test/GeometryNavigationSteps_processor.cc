#include "GeometryNavigationSteps_processor.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/Services/JGlobalRootLock.h>
#include <string>

#include "services/geometry/acts/ACTSGeo_service.h"
#include "services/rootfile/RootFile_service.h"

GeometryNavigationSteps_processor::GeometryNavigationSteps_processor(JApplication *app) :
        JEventProcessor(app)
{
}

void GeometryNavigationSteps_processor::Init()
{
    std::string plugin_name=("geometry_navigation_test");

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
    InitLogger(app, plugin_name);

    auto acts_service = GetApplication()->GetService<ACTSGeo_service>();

}


void GeometryNavigationSteps_processor::Process(const std::shared_ptr<const JEvent>& event)
{
}

void GeometryNavigationSteps_processor::Finish()
{
// Create a random number generator
  ActsExamples::RandomEngine rng =
      m_cfg.randomNumberSvc->spawnGenerator(context);

  // Standard gaussian distribution for covarianmces
  std::normal_distribution<double> gauss(0., 1.);

  // Setup random number distributions for some quantities
  std::uniform_real_distribution<double> phiDist(m_cfg.phiRange.first,
                                                 m_cfg.phiRange.second);
  std::uniform_real_distribution<double> etaDist(m_cfg.etaRange.first,
                                                 m_cfg.etaRange.second);
  std::uniform_real_distribution<double> ptDist(m_cfg.ptRange.first,
                                                m_cfg.ptRange.second);
  std::uniform_real_distribution<double> qDist(0., 1.);

  std::shared_ptr<const Acts::PerigeeSurface> surface =
      Acts::Surface::makeShared<Acts::PerigeeSurface>(
          Acts::Vector3(0., 0., 0.));

  // Output : the propagation steps
  std::vector<std::vector<Acts::detail::Step>> propagationSteps;
  propagationSteps.reserve(m_cfg.ntests);

  // Output (optional): the recorded material
  std::unordered_map<size_t, Acts::RecordedMaterialTrack> recordedMaterial;

  // loop over number of particles
  for (size_t it = 0; it < m_cfg.ntests; ++it) {
    /// get the d0 and z0
    double d0 = m_cfg.d0Sigma * gauss(rng);
    double z0 = m_cfg.z0Sigma * gauss(rng);
    double phi = phiDist(rng);
    double eta = etaDist(rng);
    double theta = 2 * atan(exp(-eta));
    double pt = ptDist(rng);
    double p = pt / sin(theta);
    double charge = qDist(rng) > 0.5 ? 1. : -1.;
    double qop = charge / p;
    double t = m_cfg.tSigma * gauss(rng);
    // parameters
    Acts::BoundVector pars;
    pars << d0, z0, phi, theta, qop, t;
    // some screen output

    Acts::Vector3 sPosition(0., 0., 0.);
    Acts::Vector3 sMomentum(0., 0., 0.);

    // The covariance generation
    auto cov = generateCovariance(rng, gauss);

    // execute the test for charged particles
    PropagationOutput pOutput;
    if (charge != 0.0) {
      // charged extrapolation - with hit recording
      Acts::BoundTrackParameters startParameters(surface, pars, std::move(cov));
      sPosition = startParameters.position(context.geoContext);
      sMomentum = startParameters.momentum();
      pOutput = m_cfg.propagatorImpl->execute(context, m_cfg, logger(),
                                              startParameters);
    } else {
      // execute the test for neutral particles
      Acts::NeutralBoundTrackParameters neutralParameters(surface, pars,
                                                          std::move(cov));
      sPosition = neutralParameters.position(context.geoContext);
      sMomentum = neutralParameters.momentum();
      pOutput = m_cfg.propagatorImpl->execute(context, m_cfg, logger(),
                                              neutralParameters);
    }
    // Record the propagator steps
    propagationSteps.push_back(std::move(pOutput.first));
    if (m_cfg.recordMaterialInteractions &&
        !pOutput.second.materialInteractions.empty()) {
      // Create a recorded material track
      RecordedMaterialTrack rmTrack;
      // Start position
      rmTrack.first.first = std::move(sPosition);
      // Start momentum
      rmTrack.first.second = std::move(sMomentum);
      // The material
      rmTrack.second = std::move(pOutput.second);
      // push it it
      recordedMaterial[it] = (std::move(rmTrack));
    }
  }

  // Write the propagation step data to the event store
  m_outpoutPropagationSteps(context, std::move(propagationSteps));

  // Write the recorded material to the event store
  if (m_cfg.recordMaterialInteractions) {
    m_recordedMaterial(context, std::move(recordedMaterial));
  }


}
