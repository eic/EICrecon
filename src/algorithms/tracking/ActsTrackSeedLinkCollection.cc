// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#include "ActsTrackSeedLinkCollection.h"

#include <podio/LinkCollection.h>

// Register the collection buffer factory so that these links can be read back
PODIO_DECLARE_LINK(ActsPodioEdm::Track, edm4eic::TrackSeed)
