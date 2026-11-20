// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#pragma once

#include <podio/ObjectID.h>

namespace eicrecon {

/*! Comparator struct for podio objects. Organizes podio objects by
 *  their ObjectID's, in increasing collection ID first, and by
 *  increasing index second.
 */
template <typename T> struct CompareObjectID;

/*! Specialization for podio::ObjectID itself. Older podio versions do
 *  not provide operator< for podio::ObjectID, so use this as the
 *  comparator of ordered containers keyed by ObjectID.
 */
template <> struct CompareObjectID<podio::ObjectID> {
  bool operator()(const podio::ObjectID& lhs, const podio::ObjectID& rhs) const {
    if (lhs.collectionID == rhs.collectionID) {
      return (lhs.index < rhs.index);
    }
    return (lhs.collectionID < rhs.collectionID);
  }
};

template <typename T> struct CompareObjectID {
  bool operator()(const T& lhs, const T& rhs) const {
    return CompareObjectID<podio::ObjectID>{}(lhs.getObjectID(), rhs.getObjectID());
  }
};

} // namespace eicrecon
