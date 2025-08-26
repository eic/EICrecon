// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#pragma once

#include <edm4eic/Cluster.h>
#include <edm4eic/Track.h>
#include <edm4eic/TrackPoint.h>
#include <map>
#include <set>
#include <vector>

namespace eicrecon {

// --------------------------------------------------------------------------
//! Particle Flow tools namespace
// --------------------------------------------------------------------------
/*! This namespace collects a variety of useful types and methods
   *  used throughout particle flow algorithms.
   */
namespace PFTools {

  // ------------------------------------------------------------------------
  //! Comparator struct for clusters
  // ------------------------------------------------------------------------
  /*! Organizes protoclusters by their ObjectID's in decreasing collection
     *  ID first, and second by decreasing index second.
     *
     *  TODO should also order by energy...
     */
  struct CompareClust {

    bool operator()(const edm4eic::Cluster& lhs, const edm4eic::Cluster& rhs) const {
      if (lhs.getObjectID().collectionID == rhs.getObjectID().collectionID) {
        return (lhs.getObjectID().index < rhs.getObjectID().index);
      } else {
        return (lhs.getObjectID().collectionID < rhs.getObjectID().collectionID);
      }
    }

  }; // end CompareCluster

  // ------------------------------------------------------------------------
  //! Convenience types
  // ------------------------------------------------------------------------
  typedef std::vector<std::vector<float>> MatrixF;
  typedef std::vector<MatrixF> VecMatrixF;
  typedef std::vector<edm4eic::Track> VecTrk;
  typedef std::vector<edm4eic::TrackPoint> VecProj;
  typedef std::vector<edm4eic::TrackSegment> VecSeg;
  typedef std::vector<edm4eic::Cluster> VecClust;
  typedef std::set<edm4eic::Cluster, CompareClust> SetClust;
  typedef std::map<edm4eic::Cluster, VecTrk, CompareClust> MapToVecTrk;
  typedef std::map<edm4eic::Cluster, VecSeg, CompareClust> MapToVecSeg;
  typedef std::map<edm4eic::Cluster, VecProj, CompareClust> MapToVecProj;
  typedef std::map<edm4eic::Cluster, VecClust, CompareClust> MapToVecClust;

} // namespace PFTools
} // namespace eicrecon
