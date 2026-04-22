// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#pragma once

#include <podio/ObjectID.h>

namespace eicrecon {

/*! Comparator struct for podio::ObjectID. Organizes podio objects by
 *  their ObjectID's in decreasing collection ID first, and second by
 *  decreasing index second.
 */
template <typename T> struct CompareObjectID {
  bool operator()(const T& lhs, const T& rhs) const {
    if (lhs.getObjectID().collectionID == rhs.getObjectID().collectionID) {
      return (lhs.getObjectID().index < rhs.getObjectID().index);
    } else {
      return (lhs.getObjectID().collectionID < rhs.getObjectID().collectionID);
    }
  }
};

} // namespace eicrecon
