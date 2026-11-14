// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#pragma once

#include <ActsPodioEdm/TrackCollection.h>
#include <edm4eic/TrackSeedCollection.h>
#include <podio/LinkCollection.h>
#include <cstddef>
#include <optional>
#include <vector>

namespace eicrecon {

/// Links each ActsPodioEdm::Track to the edm4eic::TrackSeed it was found from.
///
/// With the ActsPodioEdm backend, Acts dynamic track columns live on the track
/// container object only and are not carried by ActsPodioEdm::TrackCollection,
/// so they are lost between factories. This link collection replaces the
/// "seed" dynamic column used with the VectorTrackContainer backend.
using ActsTrackSeedLinkCollection = podio::LinkCollection<ActsPodioEdm::Track, edm4eic::TrackSeed>;
using ActsTrackSeedLink           = typename ActsTrackSeedLinkCollection::value_type;
using MutableActsTrackSeedLink    = typename ActsTrackSeedLinkCollection::mutable_type;

/// Seeds indexed by position of the linked track in `tracks`, std::nullopt for
/// tracks without a seed link. Links whose track is not in `tracks` are ignored.
inline std::vector<std::optional<edm4eic::TrackSeed>>
seedsByTrackIndex(const ActsPodioEdm::TrackCollection& tracks,
                  const ActsTrackSeedLinkCollection& links) {
  std::vector<std::optional<edm4eic::TrackSeed>> seeds(tracks.size());
  for (const auto& link : links) {
    const auto id = link.getFrom().getObjectID();
    if (id.collectionID == tracks.getID() && id.index >= 0 &&
        static_cast<std::size_t>(id.index) < seeds.size()) {
      seeds[id.index] = link.getTo();
    }
  }
  return seeds;
}

} // namespace eicrecon
