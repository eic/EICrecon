// Original licence: SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023, Sylvester Joosten, Wouter Deconinck, Dmitry Romanov, Christopher Dilks

#include "ParticlesWithPID.h"

#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/vector_utils.h>
#include <edm4hep/utils/vector_utils.h>



namespace eicrecon {

    void ParticlesWithPID::init(std::shared_ptr<spdlog::logger> logger) {
        m_log = logger;

    }

    ParticlesWithAssociation ParticlesWithPID::process(
            const edm4hep::MCParticleCollection* mc_particles,
            const edm4eic::TrajectoryCollection* trajectories,
            std::vector<const edm4eic::CherenkovParticleIDCollection*> cherenkov_pid_collections
            ) {

        // input collection

        /// Resulting reconstructed particles
        ParticlesWithAssociation out_colls;
        out_colls.parts  = std::make_unique<edm4eic::ReconstructedParticleCollection>();
        out_colls.assocs = std::make_unique<edm4eic::MCRecoParticleAssociationCollection>();
        out_colls.pids   = std::make_unique<edm4hep::ParticleIDCollection>();

        const double sinPhiOver2Tolerance = sin(0.5 * m_cfg.phiTolerance);
        tracePhiToleranceOnce(sinPhiOver2Tolerance, m_cfg.phiTolerance);

        std::vector<bool> mc_prt_is_consumed(mc_particles->size(), false);         // MCParticle is already consumed flag

        for (const auto &trajectory: *trajectories) {
          for (const auto &trk: trajectory.getTrackParameters()) {
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

                const auto p_mag = edm4eic::magnitude(p);
                const auto p_phi = edm4eic::angleAzimuthal(p);
                const auto p_eta = edm4eic::eta(p);
                const double dp_rel = std::abs((edm4eic::magnitude(mom) - p_mag) / p_mag);
                // check the tolerance for sin(dphi/2) to avoid the hemisphere problem and allow
                // for phi rollovers
                const double dsphi = std::abs(sin(0.5 * (edm4eic::angleAzimuthal(mom) - p_phi)));
                const double deta = std::abs((edm4eic::eta(mom) - p_eta));

                bool is_matching = dp_rel < m_cfg.momentumRelativeTolerance &&
                                   deta < m_cfg.etaTolerance &&
                                   dsphi < sinPhiOver2Tolerance;

                // Matching kinematics with the static variables doesn't work at low angles and within beam divergence
                // TODO - Maybe reconsider variables used or divide into regions
                // Backward going
                if ((p_eta < -5) && (edm4eic::eta(mom) < -5)) {
                  is_matching = true;
                }
                // Forward going
                if ((p_eta >  5) && (edm4eic::eta(mom) >  5)) {
                  is_matching = true;
                }

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
            auto rec_part = out_colls.parts->create();
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
            rec_part.setGoodnessOfPID(0); // assume no PID until proven otherwise
            rec_part.setPDG(best_pid);
            // rec_part.covMatrix()  // @TODO: covariance matrix on 4-momentum

            // link Cherenkov PID objects
            for (const auto& cherenkov_pids : cherenkov_pid_collections) {
                auto success = linkCherenkovPID(rec_part, *cherenkov_pids, *(out_colls.pids));
                if (success)
                    m_log->trace("      true PDG vs. CherenkovPID PDG: {:>10} vs. {:<10}",
                            best_pid,
                            rec_part.getParticleIDUsed().isAvailable() ? rec_part.getParticleIDUsed().getPDG() : 0
                            );
            }

            // Also write MC <--> truth particle association if match was found
            if (best_match >= 0) {
                auto rec_assoc = out_colls.assocs->create();
#if EDM4EIC_VERSION_MAJOR < 4
                rec_assoc.setRecID(rec_part.getObjectID().index);
                rec_assoc.setSimID((*mc_particles)[best_match].getObjectID().index);
#endif
                rec_assoc.setWeight(1);
                rec_assoc.setRec(rec_part);
                auto sim = (*mc_particles)[best_match];
                rec_assoc.setSim(sim);

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
                                 rec_assoc.getObjectID().index, rec_assoc.getSim().getObjectID().index, rec_assoc.getRec().getObjectID().index);

                    m_log->trace(" Assoc PDGs: sim.PDG | rec.PDG | rec.particleIDUsed.PDG = {:^6} | {:^6} | {:^6}",
                                 rec_assoc.getSim().getPDG(),
                                 rec_assoc.getRec().getPDG(),
                                 rec_assoc.getRec().getParticleIDUsed().isAvailable() ? rec_assoc.getRec().getParticleIDUsed().getPDG() : 0);


                }
            }
            else {
                m_log->debug(" MCPart: Did not find a good match");
            }
          }
        }

        return out_colls;
    }

    void ParticlesWithPID::tracePhiToleranceOnce(const double sinPhiOver2Tolerance, double phiTolerance) {
        // This function is called once to print tolerances useful for tracing
        static std::once_flag do_it_once;
        std::call_once(do_it_once, [this, sinPhiOver2Tolerance, phiTolerance]() {
            m_log->trace("m_cfg.phiTolerance: {:<8.4f} => sinPhiOver2Tolerance: {:<8.4f}", sinPhiOver2Tolerance, phiTolerance);
        });
    }


    /* link PID objects to input particle
     * - finds `CherenkovParticleID` object in `in_pids` associated to particle `in_part`
     *   by proximity matching to the associated track
     * - converts this `CherenkovParticleID` object's PID hypotheses to `ParticleID` objects,
     *   relates them to `in_part`, and adds them to the collection `out_pids` for persistency
     * - returns `true` iff PID objects were found and linked
     */
    bool ParticlesWithPID::linkCherenkovPID(
            edm4eic::MutableReconstructedParticle& in_part,
            const edm4eic::CherenkovParticleIDCollection& in_pids,
            edm4hep::ParticleIDCollection& out_pids
            )
    {

        // skip this particle, if neutral
        if (std::abs(in_part.getCharge()) < 0.001)
            return false;

        // structure to store list of candidate matches
        struct ProxMatch {
            double      match_dist;
            std::size_t pid_idx;
        };
        std::vector<ProxMatch> prox_match_list;

        // get input reconstructed particle momentum angles
        auto in_part_p   = in_part.getMomentum();
        auto in_part_eta = edm4hep::utils::eta(in_part_p);
        auto in_part_phi = edm4hep::utils::angleAzimuthal(in_part_p);
        m_log->trace("Input particle: (eta,phi) = ( {:>5.4}, {:>5.4} deg )",
                in_part_eta,
                in_part_phi * 180.0 / M_PI
                );

        // loop over input CherenkovParticleID objects
        for (std::size_t in_pid_idx = 0; in_pid_idx < in_pids.size(); in_pid_idx++) {
            auto in_pid = in_pids.at(in_pid_idx);

            // get charged particle track associated to this CherenkovParticleID object
            auto in_track = in_pid.getChargedParticle();
            if (!in_track.isAvailable()) {
                m_log->error("found CherenkovParticleID object with no chargedParticle");
                return false;
            }
            if (in_track.points_size() == 0) {
                m_log->error("found chargedParticle for CherenkovParticleID, but it has no TrackPoints");
                return false;
            }

            // get averge momentum direction of the track's TrackPoints
            decltype(edm4eic::TrackPoint::momentum) in_track_p{0.0, 0.0, 0.0};
            for (const auto& in_track_point : in_track.getPoints())
                in_track_p = in_track_p + ( in_track_point.momentum / in_track.points_size() );
            auto in_track_eta = edm4hep::utils::eta(in_track_p);
            auto in_track_phi = edm4hep::utils::angleAzimuthal(in_track_p);

            // calculate dist(eta,phi)
            auto match_dist = std::hypot(
                    in_part_eta - in_track_eta,
                    in_part_phi - in_track_phi
                    );

            // check if the match is close enough: within user-specified tolerances
            auto match_is_close =
                std::abs(in_part_eta - in_track_eta) < m_cfg.etaTolerance &&
                std::abs(in_part_phi - in_track_phi) < m_cfg.phiTolerance;
            if (match_is_close)
                prox_match_list.push_back(ProxMatch{match_dist, in_pid_idx});

            // logging
            m_log->trace("  - (eta,phi) = ( {:>5.4}, {:>5.4} deg ),  match_dist = {:<5.4}{}",
                    in_track_eta,
                    in_track_phi * 180.0 / M_PI,
                    match_dist,
                    match_is_close ? " => CLOSE!" : ""
                    );

        } // end loop over input CherenkovParticleID objects

        // check if at least one match was found
        if (prox_match_list.size() == 0) {
            m_log->trace("  => no matching CherenkovParticleID found for this particle");
            return false;
        }

        // choose the closest matching CherenkovParticleID object corresponding to this input reconstructed particle
        auto closest_prox_match = *std::min_element(
                prox_match_list.begin(),
                prox_match_list.end(),
                [] (ProxMatch a, ProxMatch b) { return a.match_dist < b.match_dist; }
                );
        auto in_pid_matched = in_pids.at(closest_prox_match.pid_idx);
        m_log->trace("  => best match: match_dist = {:<5.4} at idx = {}",
                closest_prox_match.match_dist,
                closest_prox_match.pid_idx
                );

        // convert `CherenkovParticleID` object's hypotheses => set of `ParticleID` objects
        auto out_pid_index_map = ConvertParticleID::ConvertToParticleIDs(in_pid_matched, out_pids, true);
        if (out_pid_index_map.size() == 0) {
            m_log->error("found CherenkovParticleID object with no hypotheses");
            return false;
        }

        // relate matched ParticleID objects to output particle
        for (const auto& [out_pids_index, out_pids_id] : out_pid_index_map) {
            const auto& out_pid = out_pids->at(out_pids_index);
            if (out_pid.id() != out_pids_id) { // sanity check
                m_log->error("indexing error in `edm4eic::ParticleID` collection");
                return false;
            }
            in_part.addToParticleIDs(out_pid);
        }
        in_part.setParticleIDUsed(in_part.getParticleIDs().at(0)); // highest likelihood is the first
        in_part.setGoodnessOfPID(1); // FIXME: not used yet, aside from 0=noPID vs 1=hasPID

        // trace logging
        m_log->trace("    {:.^50}"," PID results ");
        m_log->trace("      Hypotheses (sorted):");
        for (auto out_pid : in_part.getParticleIDs())
            Tools::PrintHypothesisTableLine(m_log, out_pid, 8);
        m_log->trace("    {:'^50}","");

        return true;
    }
}
