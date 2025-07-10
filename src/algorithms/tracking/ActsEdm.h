#pragma once

#include <podio/CollectionBase.h>

template <class edm_t>
concept is_track_container_backend = requires
{
  typename edm_t::ConstTrackContainer::TrackContainerBackend;
};

template <class edm_t>
concept is_podio_container = is_track_container_backend<edm_t> && requires (edm_t::ConstTrackContainer::TrackContainerBackend c)
{
  { c.trackCollection() } -> std::same_as<const podio::CollectionBase&>;
};
