// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024, Sylvester Joosten, Wouter Deconinck, Dmitry Romanov, Christopher Dilks, Dmitry Kalinkin

#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrajectoryCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <cmath>
#include <gsl/pointers>
#include <vector>

#include "TracksToParticles.h"

namespace eicrecon {

void TracksToParticles::init() {}

void TracksToParticles::process(const TracksToParticles::Input& input,
                                const TracksToParticles::Output& output) const {
  const auto [tracks, track_assocs] = input;
  auto [parts, part_assocs]         = output;

  for (const auto& track : *tracks) {
    auto trajectory = track.getTrajectory();
    for (const auto& trk : trajectory.getTrackParameters()) {
      const auto mom        = edm4hep::utils::sphericalToVector(1.0 / std::abs(trk.getQOverP()),
                                                                trk.getTheta(), trk.getPhi());
      const auto charge_rec = std::copysign(1., trk.getQOverP());

      debug("Converting track: index={:<4} momentum={:<8.3f} theta={:<8.3f} phi={:<8.2f} "
            "charge={:<4}",
            trk.getObjectID().index, edm4hep::utils::magnitude(mom),
            edm4hep::utils::anglePolar(mom), edm4hep::utils::angleAzimuthal(mom), charge_rec);

      auto rec_part = parts->create();
      rec_part.addToTracks(track);
      rec_part.setType(0);
      rec_part.setEnergy(edm4hep::utils::magnitude(mom));
      rec_part.setMomentum(mom);
      rec_part.setCharge(charge_rec);
      rec_part.setMass(0.);
      rec_part.setGoodnessOfPID(0); // assume no PID until proven otherwise
      // rec_part.covMatrix()  // @TODO: covariance matrix on 4-momentum

      double max_weight = -1.;
      for (auto track_assoc : *track_assocs) {
        if (track_assoc.getRec() == track) {
          trace("Found track association: index={} -> index={}, weight={}",
                track_assoc.getRec().getObjectID().index, track_assoc.getSim().getObjectID().index,
                track_assoc.getWeight());
          auto part_assoc = part_assocs->create();
          part_assoc.setRec(rec_part);
          part_assoc.setSim(track_assoc.getSim());
          part_assoc.setRecID(part_assoc.getRec().getObjectID().index);
          part_assoc.setSimID(part_assoc.getSim().getObjectID().index);
          part_assoc.setWeight(track_assoc.getWeight());

          if (max_weight < track_assoc.getWeight()) {
            max_weight                       = track_assoc.getWeight();
            edm4hep::Vector3f referencePoint = {
                static_cast<float>(track_assoc.getSim().getVertex().x),
                static_cast<float>(track_assoc.getSim().getVertex().y),
                static_cast<float>(
                    track_assoc.getSim()
                        .getVertex()
                        .z)}; // @TODO: not sure if vertex/reference point makes sense here
            rec_part.setReferencePoint(referencePoint);
          }
        }
      }
    }
  }
}
} // namespace eicrecon
