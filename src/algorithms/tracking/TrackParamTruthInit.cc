// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "TrackParamTruthInit.h"

#include <memory>
#include <vector>

#include <Acts/Definitions/Units.hpp>

#include <spdlog/spdlog.h>
#include <Acts/Surfaces/PerigeeSurface.hpp>

namespace eicrecon {

void TrackParamTruthInit::init(const std::shared_ptr<spdlog::logger> &logger) {
    m_log = logger;

}

Jug::TrackParameters *TrackParamTruthInit::produce(const edm4hep::MCParticle *part) {
    using Acts::UnitConstants::GeV;
    using Acts::UnitConstants::MeV;
    using Acts::UnitConstants::mm;
    using Acts::UnitConstants::um;
    using Acts::UnitConstants::ns;

    // getGeneratorStatus = 1 means thrown G4Primary, but dd4gun uses getGeneratorStatus == 0
    if (part->getGeneratorStatus() > 1 ) {
        m_log->trace("ignoring particle with generatorStatus = {}", part->getGeneratorStatus());
        return nullptr;
    }


    // require close to interaction vertex
    if (abs(part->getVertex().x) * mm_acts > m_maxVertexX
        || abs(part->getVertex().y) * mm_acts > m_maxVertexY
        || abs(part->getVertex().z) * mm_acts > m_maxVertexZ) {
        m_log->trace("ignoring particle with vs = {} [mm]", part->getVertex());
        }
        continue;
    }

    // require minimum momentum
    const auto& p = part->getMomentum();
    const auto pmag = std::hypot(p.x, p.y, p.z);
    if (pmag * Gaudi::Units::GeV < m_minMomentum) {
        if (msgLevel(MSG::DEBUG)) {
            debug() << "ignoring particle with p = " << pmag << " GeV" << endmsg;
        }
        continue;
    }

    // require minimum pseudorapidity
    const auto phi   = std::atan2(p.y, p.x);
    const auto theta = std::atan2(std::hypot(p.x, p.y), p.z);
    const auto eta   = -std::log(std::tan(theta/2));
    if (eta > m_maxEtaForward || eta < -std::abs(m_maxEtaBackward)) {
        if (msgLevel(MSG::DEBUG)) {
            debug() << "ignoring particle with Eta = " << eta << endmsg;
        }
        continue;
    }

    // get the particle charge
    // note that we cannot trust the mcparticles charge, as DD4hep
    // sets this value to zero! let's lookup by PDGID instead
    const double charge = m_pidSvc->particle(part->getPDG()).charge;
    if (abs(charge) < std::numeric_limits<double>::epsilon()) {
        if (msgLevel(MSG::DEBUG)) {
            debug() << "ignoring neutral particle" << endmsg;
        }
        continue;
    }



    // build some track cov matrix
    Acts::BoundSymMatrix cov                    = Acts::BoundSymMatrix::Zero();
    cov(Acts::eBoundLoc0, Acts::eBoundLoc0)     = 1000*um*1000*um;
    cov(Acts::eBoundLoc1, Acts::eBoundLoc1)     = 1000*um*1000*um;
    cov(Acts::eBoundPhi, Acts::eBoundPhi)       = 0.05*0.05;
    cov(Acts::eBoundTheta, Acts::eBoundTheta)   = 0.01*0.01;
    cov(Acts::eBoundQOverP, Acts::eBoundQOverP) = (0.1*0.1) / (GeV*GeV);
    cov(Acts::eBoundTime, Acts::eBoundTime)     = 10.0e9*ns*10.0e9*ns;

    Acts::BoundVector  params;
    params(Acts::eBoundLoc0)   = 0.0 * mm ;  // cylinder radius
    params(Acts::eBoundLoc1)   = 0.0 * mm ; // cylinder length
    params(Acts::eBoundPhi)    = phi;
    params(Acts::eBoundTheta)  = theta;
    params(Acts::eBoundQOverP) = charge / (pmag * GeV);
    params(Acts::eBoundTime)   = part->getTime() * ns;

    //// Construct a perigee surface as the target surface
    auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(
            Acts::Vector3{part->getVertex().x * mm, part->getVertex().y * mm, part->getVertex().z * mm});

    //params(Acts::eBoundQOverP) = charge/p;
    init_trk_params->push_back({pSurface, params, charge,cov});
    // std::make_optional(std::move(cov))

    if (msgLevel(MSG::DEBUG)) {
        debug() << "Invoke track finding seeded by truth particle with p = " << pmag << " GeV" << endmsg;
        debug() << "                                              charge = " << charge << endmsg;
        debug() << "                                                 q/p = " << charge / pmag << " e/GeV" << endmsg;
    }
}
}  //namespace eicrecon