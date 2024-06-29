// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "TrackParamTruthInit.h"

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Surfaces/PerigeeSurface.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/Utilities/Result.hpp>

#include <Evaluator/DD4hepUnits.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <edm4eic/Cov6f.h>
#include <fmt/core.h>
#include <spdlog/common.h>
#include <Eigen/Core>
#include <cmath>
#include <limits>
#include <memory>
#include <utility>

#include "extensions/spdlog/SpdlogFormatters.h" // IWYU pragma: keep


void eicrecon::TrackParamTruthInit::init(std::shared_ptr<const ActsGeometryProvider> geo_svc, const std::shared_ptr<spdlog::logger> logger) {
    m_log = logger;
    m_geoSvc = geo_svc;
}

std::unique_ptr<edm4eic::TrackParametersCollection>
eicrecon::TrackParamTruthInit::produce(const edm4hep::MCParticleCollection* mcparticles) {
    // MCParticles uses numerical values in its specified units,
    // while m_cfg is in the DD4hep unit system

    // Create output collection
    auto track_parameters = std::make_unique<edm4eic::TrackParametersCollection>();

    // Loop over input particles
    for (const auto& mcparticle: *mcparticles) {

        // require generatorStatus == 1 for stable generated particles in HepMC3 and DDSim gun
        if (mcparticle.getGeneratorStatus() != 1 ) {
            m_log->trace("ignoring particle with generatorStatus = {}", mcparticle.getGeneratorStatus());
            continue;
        }

        // require close to interaction vertex
        auto v = mcparticle.getVertex();
        if (abs(v.x) * dd4hep::mm > m_cfg.maxVertexX ||
            abs(v.y) * dd4hep::mm > m_cfg.maxVertexY ||
            abs(v.z) * dd4hep::mm > m_cfg.maxVertexZ) {
            m_log->trace("ignoring particle with vs = {} [mm]", v);
            continue;
        }

        // require minimum momentum
        const auto& p = mcparticle.getMomentum();
        const auto pmag = std::hypot(p.x, p.y, p.z);
        if (pmag * dd4hep::GeV < m_cfg.minMomentum) {
            m_log->trace("ignoring particle with p = {} GeV ", pmag);
            continue;
        }

        // require minimum pseudorapidity
        const auto phi   = std::atan2(p.y, p.x);
        const auto theta = std::atan2(std::hypot(p.x, p.y), p.z);
        const auto eta   = -std::log(std::tan(theta/2));
        if (eta > m_cfg.maxEtaForward || eta < -std::abs(m_cfg.maxEtaBackward)) {
            m_log->trace("ignoring particle with Eta = {}", eta);
            continue;
        }

        // get the particle charge
        // note that we cannot trust the mcparticles charge, as DD4hep
        // sets this value to zero! let's lookup by PDGID instead
        const auto pdg = mcparticle.getPDG();
        const auto particle = m_particleSvc.particle(pdg);
        double charge = std::copysign(1.0, particle.charge);
        if (abs(charge) < std::numeric_limits<double>::epsilon()) {
            m_log->trace("ignoring neutral particle");
            continue;
        }

        // modify initial momentum to avoid bleeding truth to results when fit fails
        const auto pinit = pmag * (1.0 + m_cfg.momentumSmear * m_normDist(generator));

        // define line surface for local position values
        auto perigee = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(0,0,0));

        // track particle back to transverse point-of-closest approach
        // with respect to the defined line surface
        auto linesurface_parameter = -(v.x*p.x + v.y*p.y)/(p.x*p.x + p.y*p.y);

        auto xpca = v.x + linesurface_parameter*p.x;
        auto ypca = v.y + linesurface_parameter*p.y;
        auto zpca = v.z + linesurface_parameter*p.z;

        Acts::Vector3 global(xpca, ypca, zpca);
        Acts::Vector3 direction(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta));

        // convert from global to local coordinates using the defined line surface
        auto local = perigee->globalToLocal(m_geoSvc->getActsGeometryContext(), global, direction);

        if(!local.ok())
        {
            m_log->error("skipping the track because globaltoLocal function failed");
            continue;
        }

        Acts::Vector2 localpos = local.value();

        // Insert into edm4eic::TrackParameters, which uses numerical values in its specified units
        auto track_parameter = track_parameters->create();
        track_parameter.setType(-1); // type --> seed(-1)
        track_parameter.setLoc({static_cast<float>(localpos(0)), static_cast<float>(localpos(1))}); // 2d location on surface [mm]
        track_parameter.setPhi(phi); // phi [rad]
        track_parameter.setTheta(theta); // theta [rad]
        track_parameter.setQOverP(charge / (pinit / dd4hep::GeV)); // Q/p [e/GeV]
        track_parameter.setTime(mcparticle.getTime()); // time [ns]
        edm4eic::Cov6f cov;
        cov(0,0) = 1.0; // loc0
        cov(1,1) = 1.0; // loc1
        cov(2,2) = 0.05; // phi
        cov(3,3) = 0.01; // theta
        cov(4,4) = 0.1; // qOverP
        cov(5,5) = 10e9; // time
        track_parameter.setCovariance(cov);

        // Debug output
        if (m_log->level() <= spdlog::level::debug) {
            m_log->debug("Invoke track finding seeded by truth particle with:");
            m_log->debug("   p     = {} GeV (smeared to {} GeV)", pmag / dd4hep::GeV, pinit / dd4hep::GeV);
            m_log->debug("   q     = {}", charge);
            m_log->debug("   q/p   = {} e/GeV (smeared to {} e/GeV)", charge / (pmag / dd4hep::GeV), charge / (pinit / dd4hep::GeV));
            m_log->debug("   theta = {}", theta);
            m_log->debug("   phi   = {}", phi);
        }
    }

    return std::move(track_parameters);

}
