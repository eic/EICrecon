#pragma once

#include <podio/CollectionBase.h>

// Only for Acts >= 36 can data model concepts in ActsEdm be defined.
#if Acts_VERSION_MAJOR >= 36

template <class edm_t>
concept is_track_container_backend =
    requires { typename edm_t::ConstTrackContainer::TrackContainerBackend; };

template <class edm_t>
concept is_podio_container = is_track_container_backend<edm_t> &&
                             requires(edm_t::ConstTrackContainer::TrackContainerBackend c) {
                               {
                                 c.trackCollection()
                               } -> std::derived_from<const podio::CollectionBase&>;
                             };

#endif
