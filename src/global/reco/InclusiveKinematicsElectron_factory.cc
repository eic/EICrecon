// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#include <memory>

#include <JANA/JEvent.h>

#include <spdlog/spdlog.h>

#include "InclusiveKinematicsElectron_factory.h"

#include <edm4hep/MCParticle.h>
#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/InclusiveKinematics.h>
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include <algorithms/tracking/ParticlesFromTrackFitResult.h>

namespace eicrecon {

    void InclusiveKinematicsElectron_factory::Init() {


        auto app = GetApplication();

        // This prefix will be used for parameters
        std::string param_prefix = "reco:" + GetTag();

        // Set input data tags properly
        InitDataTags(param_prefix);

        // SpdlogMixin logger initialization, sets m_log
        InitLogger(param_prefix, "info");



	if(app->GetJParameterManager()->Exists("beam:electron_energy")){
	  m_electron_beamE = app->GetParameterValue<float>("beam:electron_energy");
	}
	else{
	  m_log->info("Electron beam energy not found, using default value {}", m_electron_beamE);
	}
	
	if(app->GetJParameterManager()->Exists("beam:ion_energy")){
	  m_ion_beamE = app->GetParameterValue<float>("beam:ion_energy");
	}
	else{
	  m_log->info("Ion beam energy not found, using default value {}", m_ion_beamE);
	}
	
	if(app->GetJParameterManager()->Exists("beam:crossing_angle")){
	  m_crossingAngle = app->GetParameterValue<float>("beam:crossing_angle");
	}
	else{
	  m_log->info("Beam crossing angle not found, using default value {}", m_crossingAngle);
	}

	if(app->GetJParameterManager()->Exists("beam:ion_pdg")){
	  m_ion_pdg = app->GetParameterValue<float>("beam:ion_pdg");
	}
	else{
	  m_log->info("Ion beam species pdg not found, using default value {}", m_ion_pdg);
	}
	
        m_inclusive_kinematics_algo.init(m_log, m_electron_beamE, m_ion_beamE, m_ion_pdg, m_crossingAngle);

    }

    void InclusiveKinematicsElectron_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
        // Nothing to do here
    }

    void InclusiveKinematicsElectron_factory::Process(const std::shared_ptr<const JEvent> &event) {
        auto mc_particles = event->Get<edm4hep::MCParticle>("MCParticles");
        auto rc_particles = event->Get<edm4eic::ReconstructedParticle>("ReconstructedChargedParticles");
        auto rc_particles_assoc = event->Get<edm4eic::MCRecoParticleAssociation>("ReconstructedChargedParticleAssociations");

        auto inclusive_kinematics = m_inclusive_kinematics_algo.execute(
            mc_particles,
            rc_particles,
            rc_particles_assoc
        );

        Set(inclusive_kinematics);
    }
} // eicrecon
