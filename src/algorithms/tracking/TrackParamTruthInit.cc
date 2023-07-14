// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "TrackParamTruthInit.h"

#include <memory>
#include <vector>

#include <Acts/Definitions/Units.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <Acts/Surfaces/PerigeeSurface.hpp>

#include "extensions/spdlog/SpdlogFormatters.h"


void eicrecon::TrackParamTruthInit::init(const std::shared_ptr<spdlog::logger> &logger) {
    m_log = logger;

    // TODO make a service?
    m_pdg_db = std::make_shared<TDatabasePDG>();
}

std::unique_ptr<edm4eic::TrackParametersCollection>
eicrecon::TrackParamTruthInit::produce(const edm4hep::MCParticleCollection* mcparticles) {
    using Acts::UnitConstants::GeV;
    using Acts::UnitConstants::MeV;
    using Acts::UnitConstants::mm;
    using Acts::UnitConstants::um;
    using Acts::UnitConstants::ns;

    // Create output collection
    auto track_parameters = std::make_unique<edm4eic::TrackParametersCollection>();

    // Loop over input particles
    for (const auto& mcparticle: *mcparticles) {

        // getGeneratorStatus = 1 means thrown G4Primary, but dd4gun uses getGeneratorStatus == 0
        if (mcparticle.getGeneratorStatus() > 1 ) {
            m_log->trace("ignoring particle with generatorStatus = {}", mcparticle.getGeneratorStatus());
            continue;
        }

        // require close to interaction vertex
        if (abs(mcparticle.getVertex().x) * mm > m_cfg.m_maxVertexX
            || abs(mcparticle.getVertex().y) * mm > m_cfg.m_maxVertexY
            || abs(mcparticle.getVertex().z) * mm > m_cfg.m_maxVertexZ) {
            m_log->trace("ignoring particle with vs = {} [mm]", mcparticle.getVertex());
            continue;
        }

        // require minimum momentum
        const auto& p = mcparticle.getMomentum();
        const auto pmag = std::hypot(p.x, p.y, p.z);
        if (pmag * GeV < m_cfg.m_minMomentum) {
            m_log->trace("ignoring particle with p = {} GeV ", pmag);
            continue;
        }

        // require minimum pseudorapidity
        const auto phi   = std::atan2(p.y, p.x);
        const auto theta = std::atan2(std::hypot(p.x, p.y), p.z);
        const auto eta   = -std::log(std::tan(theta/2));
        if (eta > m_cfg.m_maxEtaForward || eta < -std::abs(m_cfg.m_maxEtaBackward)) {
            m_log->trace("ignoring particle with Eta = {}", eta);
            continue;
        }

        // get the particle charge
        // note that we cannot trust the mcparticles charge, as DD4hep
        // sets this value to zero! let's lookup by PDGID instead
        //const double charge = m_pidSvc->particle(mcparticle.getPDG()).charge;
        double charge = std::copysign(1.0,m_pdg_db->GetParticle(mcparticle.getPDG())->Charge());
        if (abs(charge) < std::numeric_limits<double>::epsilon()) {
            m_log->trace("ignoring neutral particle");
            continue;
        }

        // modify initial momentum to avoid bleeding truth to results when fit fails
        const auto pinit = pmag*(1.0 + m_cfg.m_momentumSmear * m_normDist(generator));

        auto track_parameter = track_parameters->create();
        track_parameter.setType(-1); // type --> seed(-1)
        track_parameter.setLoc({0.0 * mm, 0.0 * mm}); // 2d location on surface
        track_parameter.setLocError({1000*um, 1000*um}); // sqrt(variance) of location
        track_parameter.setTheta(theta); //theta [rad]
        track_parameter.setPhi(phi); // phi [rad]
        track_parameter.setQOverP(charge / pinit); // Q/p [e/GeV]
        track_parameter.setMomentumError({0.05,0.05,0.05}); // sqrt(variance) on theta/phi/q/p
        track_parameter.setTime(mcparticle.getTime()); // time in ns
        track_parameter.setTimeError(0.1); // error on time
        track_parameter.setCharge(charge); // charge

        // Debug output
        if (m_log->level() <= spdlog::level::debug) {
            const auto p = std::hypot(mcparticle.getMomentum().x, mcparticle.getMomentum().y,
                                      mcparticle.getMomentum().z);
            const auto theta = std::atan2(std::hypot(mcparticle.getMomentum().x,
                                                     mcparticle.getMomentum().y),
                                          mcparticle.getMomentum().z);
            const auto phi = std::atan2(mcparticle.getMomentum().y, mcparticle.getMomentum().x);
            m_log->debug("Invoke track finding seeded by truth particle with:");
            m_log->debug("   p =  {} GeV", p);
            m_log->debug("   theta = {}", theta);
            m_log->debug("   phi = {}", phi);
            m_log->debug("   charge = {}", charge);
            m_log->debug("   q/p =  {}", charge / p);
        }

    }

    return std::move(track_parameters);

}
