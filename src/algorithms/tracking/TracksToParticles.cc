// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024, Sylvester Joosten, Wouter Deconinck, Dmitry Romanov, Christopher Dilks, Dmitry Kalinkin

#include <algorithms/logger.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrajectoryCollection.h>
#include <edm4hep/ParticleIDCollection.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <stdint.h>
#include <cmath>
#include <cstddef>
#include <gsl/pointers>
#include <limits>
#include <mutex>
#include <vector>

#include "TracksToParticles.h"
#include "TracksToParticlesConfig.h"


namespace eicrecon {

    void TracksToParticles::init() {}

    void TracksToParticles::process(
      const TracksToParticles::Input& input, const TracksToParticles::Output& output
    ) const {
        const auto [mc_particles, tracks] = input;
        auto [parts, assocs]              = output;

        const double sinPhiOver2Tolerance = sin(0.5 * m_cfg.phiTolerance);
        tracePhiToleranceOnce(sinPhiOver2Tolerance, m_cfg.phiTolerance);

        std::vector<bool> mc_prt_is_consumed(mc_particles->size(), false);         // MCParticle is already consumed flag

        for (const auto &track: *tracks) {
          auto trajectory = track.getTrajectory();
          for (const auto &trk: trajectory.getTrackParameters()) {
            const auto mom = edm4hep::utils::sphericalToVector(1.0 / std::abs(trk.getQOverP()), trk.getTheta(),
                                                        trk.getPhi());
            const auto charge_rec = std::copysign(1., trk.getQOverP());


            debug("Match:  [id]   [mom]   [theta]  [phi]    [charge]  [PID]");
            debug(" Track : {:<4} {:<8.3f} {:<8.3f} {:<8.2f} {:<4}",
                         trk.getObjectID().index, edm4hep::utils::magnitude(mom), edm4hep::utils::anglePolar(mom), edm4hep::utils::angleAzimuthal(mom), charge_rec);

            // utility variables for matching
            int best_match = -1;
            double best_delta = std::numeric_limits<double>::max();
            for (size_t ip = 0; ip < mc_particles->size(); ++ip) {
                const auto &mc_part = (*mc_particles)[ip];
                const auto &p = mc_part.getMomentum();

                trace("  MCParticle with id={:<4} mom={:<8.3f} charge={}", mc_part.getObjectID().index,
                             edm4hep::utils::magnitude(p), mc_part.getCharge());

                // Check if used
                if (mc_prt_is_consumed[ip]) {
                    trace("    Ignoring. Particle is already used");
                    continue;
                }

                // Check if non-primary
                if (mc_part.getGeneratorStatus() > 1) {
                    trace("    Ignoring. GeneratorStatus > 1 => Non-primary particle");
                    continue;
                }

                // Check if neutral
                if (mc_part.getCharge() == 0) {
                    trace("    Ignoring. Neutral particle");
                    continue;
                }

                // Check opposite charge
                if (mc_part.getCharge() * charge_rec < 0) {
                    trace("    Ignoring. Opposite charge particle");
                    continue;
                }

                const auto p_mag = edm4hep::utils::magnitude(p);
                const auto p_phi = edm4hep::utils::angleAzimuthal(p);
                const auto p_eta = edm4hep::utils::eta(p);
                const double dp_rel = std::abs((edm4hep::utils::magnitude(mom) - p_mag) / p_mag);
                // check the tolerance for sin(dphi/2) to avoid the hemisphere problem and allow
                // for phi rollovers
                const double dsphi = std::abs(sin(0.5 * (edm4hep::utils::angleAzimuthal(mom) - p_phi)));
                const double deta = std::abs((edm4hep::utils::eta(mom) - p_eta));

                bool is_matching = dp_rel < m_cfg.momentumRelativeTolerance &&
                                   deta < m_cfg.etaTolerance &&
                                   dsphi < sinPhiOver2Tolerance;

                // Matching kinematics with the static variables doesn't work at low angles and within beam divergence
                // TODO - Maybe reconsider variables used or divide into regions
                // Backward going
                if ((p_eta < -5) && (edm4hep::utils::eta(mom) < -5)) {
                  is_matching = true;
                }
                // Forward going
                if ((p_eta >  5) && (edm4hep::utils::eta(mom) >  5)) {
                  is_matching = true;
                }

                trace("    Decision: {}  dp: {:.4f} < {}  &&  d_eta: {:.6f} < {}  && d_sin_phi: {:.4e} < {:.4e} ",
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
                        trace("    Is the best match now");
                    }
                }
            }
            auto rec_part = parts->create();
            rec_part.addToTracks(track);
            auto referencePoint = rec_part.getReferencePoint();
            if (best_match >= 0) {
                trace("Best match is found and is: {}", best_match);
                mc_prt_is_consumed[best_match] = true;
                const auto &best_mc_part = (*mc_particles)[best_match];
                referencePoint = {
                        static_cast<float>(best_mc_part.getVertex().x),
                        static_cast<float>(best_mc_part.getVertex().y),
                        static_cast<float>(best_mc_part.getVertex().z)}; // @TODO: not sure if vertex/reference point makes sense here
            }

            rec_part.setType(static_cast<int16_t>(best_match >= 0 ? 0 : -1)); // @TODO: determine type codes
            rec_part.setEnergy(edm4hep::utils::magnitude(mom));
            rec_part.setMomentum(mom);
            rec_part.setReferencePoint(referencePoint);
            rec_part.setCharge(charge_rec);
            rec_part.setMass(0.);
            rec_part.setGoodnessOfPID(0); // assume no PID until proven otherwise
            // rec_part.covMatrix()  // @TODO: covariance matrix on 4-momentum

            // Also write MC <--> truth particle association if match was found
            if (best_match >= 0) {
                auto rec_assoc = assocs->create();
                rec_assoc.setRecID(rec_part.getObjectID().index);
                rec_assoc.setSimID((*mc_particles)[best_match].getObjectID().index);
                rec_assoc.setWeight(1);
                rec_assoc.setRec(rec_part);
                auto sim = (*mc_particles)[best_match];
                rec_assoc.setSim(sim);

                if (level() <= algorithms::LogLevel::kDebug) {

                    const auto &mcpart = (*mc_particles)[best_match];
                    const auto &p = mcpart.getMomentum();
                    const auto p_mag = edm4hep::utils::magnitude(p);
                    const auto p_phi = edm4hep::utils::angleAzimuthal(p);
                    const auto p_theta = edm4hep::utils::anglePolar(p);
                    debug(" MCPart: {:<4} {:<8.3f} {:<8.3f} {:<8.2f} {:<6}",
                                 mcpart.getObjectID().index, p_mag, p_theta, p_phi, mcpart.getCharge(),
                                 mcpart.getPDG());

                    debug(" Assoc: id={} SimId={} RecId={}",
                                 rec_assoc.getObjectID().index, rec_assoc.getSim().getObjectID().index, rec_assoc.getSim().getObjectID().index);

                    trace(" Assoc PDGs: sim.PDG | rec.PDG | rec.particleIDUsed.PDG = {:^6} | {:^6} | {:^6}",
                                 rec_assoc.getSim().getPDG(),
                                 rec_assoc.getRec().getPDG(),
                                 rec_assoc.getRec().getParticleIDUsed().isAvailable() ? rec_assoc.getRec().getParticleIDUsed().getPDG() : 0);


                }
            }
            else {
                debug(" MCPart: Did not find a good match");
            }
          }
        }
    }

    void TracksToParticles::tracePhiToleranceOnce(const double sinPhiOver2Tolerance, double phiTolerance) const {
        // This function is called once to print tolerances useful for tracing
        static std::once_flag do_it_once;
        std::call_once(do_it_once, [this, sinPhiOver2Tolerance, phiTolerance]() {
            trace("m_cfg.phiTolerance: {:<8.4f} => sinPhiOver2Tolerance: {:<8.4f}", sinPhiOver2Tolerance, phiTolerance);
        });
    }
}
