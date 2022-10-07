// Original licence: SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Wouter Deconinck, Dmitry Romanov

#include <algorithm>
#include <cmath>

#include <fmt/format.h>

#include <spdlog/spdlog.h>

// Event Model related classes
#include "edm4hep/MCParticleCollection.h"
#include "edm4eic/MCRecoParticleAssociationCollection.h"
#include "edm4eic/ReconstructedParticleCollection.h"
#include "edm4eic/TrackParametersCollection.h"
#include "edm4eic/vector_utils.h"

#include "ParticlesWithTruthPIDConfig.h"
#include "ParticlesWithTruthPID.h"
#include "algorithms/reco/ParticlesWithAssociation.h"
#include <algorithms/interfaces/WithPodConfig.h>

namespace eicrecon {


    ParticlesWithAssociation * ParticlesWithTruthPID::execute(
            const std::vector<const edm4hep::MCParticle *> &mc_particles,
            const std::vector<const edm4eic::TrackParameters*> &track_params) {
        // input collection

        /// Resulting reconstructed particles
        std::vector<edm4eic::ReconstructedParticle *> reco_particles;

        /// Resulting associations
        std::vector<edm4eic::MCRecoParticleAssociation *> associations;


        const double sinPhiOver2Tolerance = sin(0.5 * m_cfg.phiTolerance);
        std::vector<bool> consumed(mc_particles.size(), false);
        for (const auto &trk: track_params) {
            const auto mom = edm4eic::sphericalToVector(1.0 / std::abs(trk->getQOverP()), trk->getTheta(), trk->getPhi());
            const auto charge_rec = trk->getCharge();
            // utility variables for matching
            int best_match = -1;
            double best_delta = std::numeric_limits<double>::max();
            for (size_t ip = 0; ip < mc_particles.size(); ++ip) {
                const auto &mc_part = mc_particles[ip];
                if (consumed[ip] || mc_part->getGeneratorStatus() > 1 || mc_part->getCharge() == 0 ||
                    mc_part->getCharge() * charge_rec < 0) {
                    m_log->debug("ignoring non-primary/neutral/opposite charge particle");
                    continue;
                }
                const auto &p = mc_part->getMomentum();
                const auto p_mag = std::hypot(p.x, p.y, p.z);
                const auto p_phi = std::atan2(p.y, p.x);
                const auto p_eta = std::atanh(p.z / p_mag);
                const double dp_rel = std::abs((edm4eic::magnitude(mom) - p_mag) / p_mag);
                // check the tolerance for sin(dphi/2) to avoid the hemisphere problem and allow
                // for phi rollovers
                const double dsphi = std::abs(sin(0.5 * (edm4eic::angleAzimuthal(mom) - p_phi)));
                const double deta = std::abs((edm4eic::eta(mom) - p_eta));

                if (dp_rel < m_cfg.pRelativeTolerance && deta < m_cfg.etaTolerance && dsphi < sinPhiOver2Tolerance) {
                    const double delta =
                            std::hypot(dp_rel / m_cfg.pRelativeTolerance, deta / m_cfg.etaTolerance,
                                       dsphi / sinPhiOver2Tolerance);
                    if (delta < best_delta) {
                        best_match = ip;
                        best_delta = delta;
                    }
                }
            }
            auto rec_part = edm4eic::MutableReconstructedParticle();
            int32_t best_pid = 0;
            auto referencePoint = rec_part.referencePoint();
            // float time          = 0;
            float mass = 0;
            if (best_match >= 0) {
                consumed[best_match] = true;
                const auto &best_mc_part = mc_particles[best_match];
                best_pid = best_mc_part->getPDG();
                referencePoint = {
                        static_cast<float>(best_mc_part->getVertex().x), static_cast<float>(best_mc_part->getVertex().y),
                        static_cast<float>(best_mc_part->getVertex().z)}; // @TODO: not sure if vertex/reference point makes sense here
                // time                 = mcpart.getTime();
                mass = best_mc_part->getMass();
            }
            rec_part.setType(static_cast<int16_t>(best_match >= 0 ? 0 : -1)); // @TODO: determine type codes
            rec_part.setEnergy(std::hypot(edm4eic::magnitude(mom), mass));
            rec_part.setMomentum(mom);
            rec_part.setReferencePoint(referencePoint);
            rec_part.setCharge(charge_rec);
            rec_part.setMass(mass);
            rec_part.setGoodnessOfPID(1); // perfect PID
            rec_part.setPDG(best_pid);
            // rec_part.covMatrix()  // @TODO: covariance matrix on 4-momentum
            // Also write MC <--> truth particle association if match was found
            if (best_match >= 0) {
                auto rec_assoc = edm4eic::MutableMCRecoParticleAssociation();
                rec_assoc.setRecID(rec_part.getObjectID().index);
                rec_assoc.setSimID(mc_particles[best_match]->getObjectID().index);
                rec_assoc.setWeight(1);
                rec_assoc.setRec(rec_part);
                //rec_assoc.setSim(mc[best_match]);
                associations.emplace_back(new edm4eic::MCRecoParticleAssociation(rec_assoc));
            }
            if (m_log->level() <= spdlog::level::debug) {
                if (best_match > 0) {
                    const auto &mcpart = mc_particles[best_match];
                    m_log->debug("Matched track with MC particle {}\n", best_match);
                    m_log->debug("  - Track: (mom: {}, theta: {}, phi: {}, charge: {})",
                                 edm4eic::magnitude(mom),
                                 edm4eic::anglePolar(mom), edm4eic::angleAzimuthal(mom), charge_rec);
                    const auto &p = mcpart->getMomentum();
                    const auto p_mag = edm4eic::magnitude(p);
                    const auto p_phi = edm4eic::angleAzimuthal(p);
                    const auto p_theta = edm4eic::anglePolar(p);
                    m_log->debug("  - MC particle: (mom: {}, theta: {}, phi: {}, charge: {}, type: {}",
                                 p_mag, p_theta,
                                 p_phi, mcpart->getCharge(), mcpart->getPDG());
                } else {
                    m_log->debug("Did not find a good match for track \n");
                    m_log->debug("  - Track: (mom: {}, theta: {}, phi: {}, charge: {})",
                                 edm4eic::magnitude(mom),
                                 edm4eic::anglePolar(mom), edm4eic::angleAzimuthal(mom), charge_rec);
                }
            }

            // Add particle to the output vector
            reco_particles.push_back(new edm4eic::ReconstructedParticle(rec_part));
        }

        // Assembling the results
        return new ParticlesWithAssociation(std::move(reco_particles), std::move(associations));
    }

}