// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "TruthTrackSeeding.h"

#include <JANA/JException.h>
#include <TDatabasePDG.h>

void eicrecon::TruthTrackSeeding::init() {

    // TODO make a service?
    m_pdg_db = std::make_shared<TDatabasePDG>();

}

eicd::TrackParameters *eicrecon::TruthTrackSeeding::produce(const edm4hep::MCParticle *part) {

    // getGeneratorStatus = 1 means thrown G4Primary
    if(part->getGeneratorStatus() != 1 ) {
        return nullptr;
    }

    const auto& pvec = part->getMomentum();
    const auto p = std::hypot(pvec.x, pvec.y, pvec.z);
    const auto phi = std::atan2(pvec.x, pvec.y);
    const auto theta = std::atan2(std::hypot(pvec.x, pvec.y), pvec.z);

    // get the particle charge
    // note that we cannot trust the mcparticles charge, as DD4hep
    // sets this value to zero! let's lookup by PDGID instead
    //TODO const auto charge = static_cast<float>(m_pidSvc->particle().charge);
    float charge =m_pdg_db->GetParticle(part->getPDG())->Charge();

    if (fabs(charge) < std::numeric_limits<float>::epsilon()) {
        return nullptr;
    }

    float q_over_p = charge / p;

    auto params = new eicd::TrackParameters {-1,                // type --> seed (-1)
                                 {0.0F, 0.0F},      // location on surface
                                 {0.1, 0.1, 0.1},   // Covariance on location
                                 theta,             // theta (rad)
                                 phi,               // phi  (rad)
                                 q_over_p * .05F,   // Q/P (e/GeV)
                                 {0.1, 0.1, 0.1},   // Covariance on theta/phi/Q/P
                                 part->getTime(),    // Time (ns)
                                 0.1,               // Error on time
                                 charge};           // Charge

    ////// Construct a perigee surface as the target surface
    //auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(
    //    Acts::Vector3{part.getVertex().x * mm, part.getVertex().y * mm, part.getVertex().z * mm});

    return params;
}
