// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson

#include <edm4eic/CalorimeterHit.h>
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

#include "TrackClusterSubtractor.h"
#include "algorithms/particle/TrackClusterSubtractorConfig.h"



namespace eicrecon {

  // --------------------------------------------------------------------------
  //! Initialize algorithm
  // --------------------------------------------------------------------------
  void TrackClusterSubtractor::init() {

    //... nothing to do ...//

  }  // end 'init(dd4hep::Detector*)'



  // --------------------------------------------------------------------------
  //! Process inputs
  // --------------------------------------------------------------------------
  /*! Subtract energy of matched tracks via the following algorithm.
   *    1. Build a map of each cluster onto a list of matched
   *       track projections.
   *    2. For each cluster, subtract the sum of momenta of
   *       all matched tracks scaled by the specified fraction
   *       from the cluster's energy.
   *    3. If subtracted energy is greater than 0, copy cluster
   *       into output with new subtracted energy.
   */
  void TrackClusterSubtractor::process(
    const TrackClusterSubtractor::Input& input,
    const TrackClusterSubtractor::Output& output
  ) const {

    // grab inputs/outputs
    const auto [in_matches, in_projections] = input;
    auto [out_clusters, out_matches] = output;

    // exit if no matched tracks in collection
    if (in_matches->size() == 0) {
      debug("No matched tracks in collection.");
      return;
    }

    // ------------------------------------------------------------------------
    // 1. Build map of clusters onto projections
    // ------------------------------------------------------------------------
    PFTools::MapToVecSeg mapClustToProj;
    for (const auto& match : *in_matches) {
      for (const auto& project : *in_projections) {

        // pick out corresponding projection from track
        if (match.getTrack() != project.getTrack()) {
          continue;
        } else {
          mapClustToProj[ match.getCluster() ].push_back( project );
        }

      }  // end projection loop
    }  // end track-cluster match loop
    debug("Built map of clusters-onto-tracks, size = {}", mapClustToProj.size());

    // ------------------------------------------------------------------------
    // 2. Subtract energy for tracks
    // ------------------------------------------------------------------------ 
    // FIXME need to think: for charged particles, energy reconstruction
    // should use only the portion of energy relevant to the charged
    // track if there is something leftover after subtraction...
    for (const auto& [cluster, projects] : mapClustToProj) {

      // do subtraction
      const double eTrkSum = sum_track_energy(projects);
      const double eToSub = m_cfg.fracEnergyToSub * eTrkSum;
      const double eSub = cluster.getEnergy() - eToSub;
      trace("Subtracted {} GeV from cluster with {} GeV", eToSub, cluster.getEnergy());

      // if greater than 0, scale energy accordingly and
      // write out remnant cluster
      if (is_zero(eSub)) {
        continue;
      }
      const double scale = eSub / cluster.getEnergy();

      // update cluster energy
      edm4eic::MutableCluster remnant_clust = cluster.clone();
      remnant_clust.setEnergy( scale * cluster.getEnergy() );
      out_clusters->push_back(remnant_clust);
      trace("Create remnant cluster with {} GeV", remnant_clust.getEnergy());

    }  // end cluster-to-projections loop
    debug("Finished subtraction, {} clusters leftover", out_clusters->size());

  }  // end 'get_projections(edm4eic::CalorimeterHit&, edm4eic::TrackSegmentCollection&, VecTrkPoint&)'



  // --------------------------------------------------------------------------
  //! Sum energy of tracks
  // --------------------------------------------------------------------------
  /*! Sums energy of tracks projected to the surface in the
   *  calorimeter specified by `surfaceToUse`. Uses PDG of
   *  track to select mass for energy; if not available,
   *  uses mass set by `defaultMassPdg`.
   */
  double TrackClusterSubtractor::sum_track_energy(const PFTools::VecSeg& projects) const {

    double eSum = 0.;
    for (const auto& project : projects) {

      // measure momentum at specified surface
      double momentum = 0.;
      for (const auto& point : project.getPoints()) {
        if (point.surface != m_cfg.surfaceToUse) {
          continue;
        } else {
          momentum = edm4hep::utils::magnitude( point.momentum );
          break;
        }
      }

      // get mass based on track pdg
      double mass = m_parSvc.particle(m_cfg.defaultMassPdg).mass;
      if (project.getTrack().getPdg() != 0) {
        mass = m_parSvc.particle(project.getTrack().getPdg()).mass;
      }

      // increment sum
      eSum += std::sqrt((momentum*momentum) + (mass*mass));
    }

    // output debugging and exit
    trace("Sum of track energy = {} GeV", eSum);
    return eSum;

  }  // end 'sum_track_energy(PFTools::VecSeg&)'



  // --------------------------------------------------------------------------
  //! Is difference consistent with zero?
  // --------------------------------------------------------------------------
  /*! Checks if provided difference is consistent with zero,
   *  either checking if difference is within an epsilon
   *  (if `doNSigmaCut` is false), or if difference is within
   *  `nSigmaMax` of zero (if `doNSigmaCut` is true) based on
   *  the provided tracker and calorimeter resolutions.
   */
  bool TrackClusterSubtractor::is_zero(const double difference) const {

    // if < 0, automatically return true
    if (difference < 0) {
      return true;
    }

    // calculate nSigma
    const double totReso = std::sqrt((m_cfg.trkReso*m_cfg.trkReso) + (m_cfg.calReso*m_cfg.calReso));
    const double nSigma = difference / totReso;

    // do appropriate comparison
    bool isZero = false;
    if (m_cfg.doNSigmaCut) {
      isZero = (nSigma < m_cfg.nSigmaMax);
      trace(
        "Difference of {} GeV consistent with zero: nSigma = {} < {}",
        difference,
        nSigma,
        m_cfg.nSigmaMax
      );
    } else {
      isZero = std::abs(difference) < std::numeric_limits<double>::epsilon();
      trace(
        "Difference of {} GeV consistent with zero within an epsilon",
        difference
      );
    }
    return isZero;

  }  // end 'is_zero(double)'

}  // end eicrecon namespace
