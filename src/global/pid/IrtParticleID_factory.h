// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

/* - input: photoelectrons, tracking info
 * - output: Cherenkov PID hypothesis and emission angle
 * - prepares for and calls the IRT (standalone) algorithm
 */

#pragma once

#include <cmath>

// JANA
#include <JANA/JEvent.h>
#include <services/io/podio/JFactoryPodioT.h>

// data model
#include <edm4hep/SimTrackerHit.h>
#include <edm4hep/ParticleID.h>

// services
#include <services/geometry/richgeo/RichGeo_service.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

namespace eicrecon {
  class IrtParticleID_factory : public eicrecon::JFactoryPodioT<edm4hep::ParticleID> {
    public:

      IrtParticleID_factory() {
        SetTag("IrtHypothesis"); // FIXME: should be D/PFRICH-dependent name?
      }

      /** One time initialization **/
      void Init() override;

      /** On run change preparations **/
      void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

      /** Event by event processing **/
      void Process(const std::shared_ptr<const JEvent> &event) override;

    private:
      std::string m_detector_name;
      std::shared_ptr<RichGeo_service> m_richGeoSvc;
      std::shared_ptr<spdlog::logger> m_log;
      CherenkovDetectorCollection *m_irtDetectorCollection;

  };
}
