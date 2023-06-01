// Original licence: SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Wouter Deconinck, Dmitry Romanov

#include "ParticlesWithTruthPID.h"

#include <edm4eic/vector_utils.h>



namespace eicrecon {

    void ParticlesWithTruthPID::init(std::shared_ptr<spdlog::logger> logger) {
        m_log = logger;

    }

    ParticlesWithAssociationNew ParticlesWithTruthPID::process(
            const edm4hep::MCParticleCollection* mc_particles,
            const edm4eic::TrackParametersCollection* track_params) {

        // input collection

        /// Resulting reconstructed particles
        auto reco_particles = std::make_unique<edm4eic::ReconstructedParticleCollection>();
        auto associations = std::make_unique<edm4eic::MCRecoParticleAssociationCollection>();

        const double sinPhiOver2Tolerance = sin(0.5 * m_cfg.phiTolerance);
        tracePhiToleranceOnce(sinPhiOver2Tolerance, m_cfg.phiTolerance);

        std::vector<bool> mc_prt_is_consumed(mc_particles->size(), false);         // MCParticle is already consumed flag

        for (const auto &trk: *track_params) {
            const auto mom = edm4eic::sphericalToVector(1.0 / std::abs(trk.getQOverP()), trk.getTheta(),
                                                        trk.getPhi());
            const auto charge_rec = trk.getCharge();


            m_log->debug("Match:  [id]   [mom]   [theta]  [phi]    [charge]  [PID]");
            m_log->debug(" Track : {:<4} {:<8.3f} {:<8.3f} {:<8.2f} {:<4}",
                         trk.getObjectID().index, edm4eic::magnitude(mom), edm4eic::anglePolar(mom), edm4eic::angleAzimuthal(mom), charge_rec);

            // utility variables for matching
            int best_match = -1;
            double best_delta = std::numeric_limits<double>::max();
            for (size_t ip = 0; ip < mc_particles->size(); ++ip) {
                const auto &mc_part = (*mc_particles)[ip];
                const auto &p = mc_part.getMomentum();

                m_log->trace("  MCParticle with id={:<4} mom={:<8.3f} charge={}", mc_part.getObjectID().index,
                             edm4eic::magnitude(p), mc_part.getCharge());

                // Check if used
                if (mc_prt_is_consumed[ip]) {
                    m_log->trace("    Ignoring. Particle is already used");
                    continue;
                }

                // Check if non-primary
                if (mc_part.getGeneratorStatus() > 1) {
                    m_log->trace("    Ignoring. GeneratorStatus > 1 => Non-primary particle");
                    continue;
                }

                // Check if neutral
                if (mc_part.getCharge() == 0) {
                    m_log->trace("    Ignoring. Neutral particle");
                    continue;
                }

                // Check opposite charge
                if (mc_part.getCharge() * charge_rec < 0) {
                    m_log->trace("    Ignoring. Opposite charge particle");
                    continue;
                }

                const auto p_mag = std::hypot(p.x, p.y, p.z);
                const auto p_phi = std::atan2(p.y, p.x);
                const auto p_eta = std::atanh(p.z / p_mag);
                const double dp_rel = std::abs((edm4eic::magnitude(mom) - p_mag) / p_mag);
                // check the tolerance for sin(dphi/2) to avoid the hemisphere problem and allow
                // for phi rollovers
                const double dsphi = std::abs(sin(0.5 * (edm4eic::angleAzimuthal(mom) - p_phi)));
                const double deta = std::abs((edm4eic::eta(mom) - p_eta));

                bool is_matching = dp_rel < m_cfg.momentumRelativeTolerance &&
                                   deta < m_cfg.etaTolerance &&
                                   dsphi < sinPhiOver2Tolerance;

                m_log->trace("    Decision: {}  dp: {:.4f} < {}  &&  d_eta: {:.6f} < {}  && d_sin_phi: {:.4e} < {:.4e} ",
                             is_matching? "Matching":"Ignoring",
                             dp_rel, m_cfg.momentumRelativeTolerance,
                             deta, m_cfg.etaTolerance,
                             dsphi, sinPhiOver2Tolerance);

                if (is_matching) {
                    const double delta =
                            std::hypot(dp_rel / m_cfg.momentumRelativeTolerance, deta / m_cfg.etaTolerance,
                                       dsphi / sinPhiOver2Tolerance);
                    if (delta < best_delta) {
                        best_match = ip;
                        best_delta = delta;
                        m_log->trace("    Is the best match now");
                    }
                }
            }
            auto rec_part = edm4eic::MutableReconstructedParticle();
            int32_t best_pid = 0;
            auto referencePoint = rec_part.referencePoint();
            // float time          = 0;
            float mass = 0;
            if (best_match >= 0) {
                m_log->trace("Best match is found and is: {}", best_match);
                mc_prt_is_consumed[best_match] = true;
                const auto &best_mc_part = (*mc_particles)[best_match];
                best_pid = best_mc_part.getPDG();
                referencePoint = {
                        static_cast<float>(best_mc_part.getVertex().x),
                        static_cast<float>(best_mc_part.getVertex().y),
                        static_cast<float>(best_mc_part.getVertex().z)}; // @TODO: not sure if vertex/reference point makes sense here
                // time                 = mcpart.getTime();
                mass = best_mc_part.getMass();
            }

            rec_part.setType(static_cast<int16_t>(best_match >= 0 ? 0 : -1)); // @TODO: determine type codes
            rec_part.setEnergy((float) std::hypot(edm4eic::magnitude(mom), mass));
            rec_part.setMomentum(mom);
            rec_part.setReferencePoint(referencePoint);
            rec_part.setCharge(charge_rec);
            rec_part.setMass(mass);
            rec_part.setGoodnessOfPID(1); // perfect PID
            rec_part.setPDG(best_pid);
            // rec_part.covMatrix()  // @TODO: covariance matrix on 4-momentum
            // Add reconstructed particle to collection BEFORE doing association
            reco_particles->push_back(rec_part);

            // Also write MC <--> truth particle association if match was found
            if (best_match >= 0) {
                auto rec_assoc = edm4eic::MutableMCRecoParticleAssociation();
                rec_assoc.setRecID(rec_part.getObjectID().index);
                rec_assoc.setSimID((*mc_particles)[best_match].getObjectID().index);
                rec_assoc.setWeight(1);
                rec_assoc.setRec(rec_part);
                auto sim = (*mc_particles)[best_match];
                rec_assoc.setSim(sim);

                // Add association to collection
                associations->push_back(rec_assoc);

                if (m_log->level() <= spdlog::level::debug) {

                    const auto &mcpart = (*mc_particles)[best_match];
                    const auto &p = mcpart.getMomentum();
                    const auto p_mag = edm4eic::magnitude(p);
                    const auto p_phi = edm4eic::angleAzimuthal(p);
                    const auto p_theta = edm4eic::anglePolar(p);
                    m_log->debug(" MCPart: {:<4} {:<8.3f} {:<8.3f} {:<8.2f} {:<6}",
                                 mcpart.getObjectID().index, p_mag, p_theta, p_phi, mcpart.getCharge(),
                                 mcpart.getPDG());

                    m_log->debug(" Assoc: id={} SimId={} RecId={}",
                                 rec_assoc.getObjectID().index, rec_assoc.getSimID(), rec_assoc.getSimID());
                }
            }
            else {
                m_log->debug(" MCPart: Did not find a good match");
            }

        }

        // Assembling the results
        return std::make_pair(std::move(reco_particles), std::move(associations));
    }

    void ParticlesWithTruthPID::tracePhiToleranceOnce(const double sinPhiOver2Tolerance, double phiTolerance) {
        // This function is called once to print tolerances useful for tracing
        static std::once_flag do_it_once;
        std::call_once(do_it_once, [this, sinPhiOver2Tolerance, phiTolerance]() {
            m_log->trace("m_cfg.phiTolerance: {:<8.4f} => sinPhiOver2Tolerance: {:<8.4f}", sinPhiOver2Tolerance, phiTolerance);
        });
    }
}
