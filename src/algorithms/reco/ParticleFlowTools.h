// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Derek Anderson

#pragma once

// c++ utilities
#include <set>
#include <vector>
#include <limits>
#include <optional>
#include <algorithm>
#include <functional>
#include <JANA/JException.h>
#include <DD4hep/Detector.h>
#include <DD4hep/DetElement.h>
// event data model definitions
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <edm4eic/Cluster.h>
#include <edm4eic/Track.h>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegment.h>

// ----------------------------------------------------------------------------
//! PFTools Namespace
// ----------------------------------------------------------------------------
/*! A namespace to collect various utilities used in
 *  the various particle flow algorithms.
 */
namespace eicrecon {
  namespace PFTools {

    /*! Various constants used in particle flow algorithms.
     *
     *  TODO these might be better handled in a
     *  PDG database rather than being hard-coded...
     */
    struct Constants {
      uint64_t innerSurface;
      int32_t  idPi0;
      int32_t  idPiP;
      float    massPiCharged;
      float    massPi0;
    } consts = {1, 111, 211, 0.140, 0.135};



    /*! Comparator to input track projections in decreasing momentum.
     */
    struct is_project_mom_greater_than {
      bool operator() (const edm4eic::TrackSegment& lhs, const edm4eic::TrackSegment& rhs) const {
        return (edm4hep::utils::magnitude(lhs.getTrack().getMomentum()) > edm4hep::utils::magnitude(rhs.getTrack().getMomentum()));
      }
    };



    /*! Comparator to sort input clsuters in decreasing energy.
     */
    struct is_clust_ene_greater_than {
      bool operator() (const edm4eic::Cluster& lhs, const edm4eic::Cluster& rhs) const {
        return (lhs.getEnergy() > rhs.getEnergy());
      }
    };



    // ------------------------------------------------------------------------
    //! Get ID of Detector Subsystem
    // ------------------------------------------------------------------------
    /*! Returns system ID of detector subsystem specified by `name` (e.g.
        "HcalBarrel").  Throws exception is subsystem is unable to be located.
     */
    uint32_t get_detector_id(const dd4hep::Detector* detector, const std::string name) {

      dd4hep::DetElement element;
      try {
        element = detector -> detector(name.data());
      } catch (...) {
        throw JException("unknown detector name");
      }
      return element.id();

    }  // end 'get_detector_id(dd4hep::Detector*, std::string)



    // ------------------------------------------------------------------------
    //! Is Track Pointing to a System?
    // ------------------------------------------------------------------------
    /*! Returns true if a track projection is pointing to one of the provided
     *  systems (i.e. detector volumes) based on their system IDs.  Returns
     *  false otherwise.
     */ 
    bool is_track_pointing_to_system(const edm4eic::TrackPoint& point, std::vector<uint32_t> sysToConsider) {

      bool isPointingToSys = false;
      for (const auto sys : sysToConsider) {
        if (point.system == sys) {
          isPointingToSys = true;
          break;
        }
      }
      return isPointingToSys;

    }  // end 'is_track_pointing_to_system(edm4eic::TrackPoint&, std::vector<uint32_t>)'



    // ------------------------------------------------------------------------
    //! Calculate Distance in Eta-Phi Between Two Points
    // ------------------------------------------------------------------------
    /*! Calculates distance in the eta-phi plane between two points provided
     *  in cartesian coordinates.
     */
    float calculate_dist_in_eta_phi(const edm4hep::Vector3f& pntA, const edm4hep::Vector3f& pntB) {

      ROOT::Math::XYZPoint xyzA(pntA.x, pntA.y, pntA.z);
      ROOT::Math::XYZPoint xyzB(pntB.x, pntB.y, pntB.z);

      // translate cartesian coordinates to cylindrical
      ROOT::Math::RhoEtaPhiPoint rhfA(xyzA);
      ROOT::Math::RhoEtaPhiPoint rhfB(xyzB);

      // calculate distance and return
      const float dist = std::hypot(rhfA.eta() - rhfB.eta(), rhfA.phi() - rhfA.phi());
      return dist;

    }  // end 'calculate_dist_in_eta_phi(edm4hep::Vector3f&, edm4hep::Vector3f&)'



    // ------------------------------------------------------------------------
    //! Calculate Energy at Point
    // ------------------------------------------------------------------------
    /*! Calculates energy of a track projection at a provided
     *  point using a provided mass.
     */
    float calculate_energy_at_point(const edm4eic::TrackPoint& point, const float mass = 0.) {

      const float momentum = edm4hep::utils::magnitude(point.momentum);
      const float energy   = (momentum > mass) ? std::hypot(momentum, mass) : 0.;
      return energy;

    }  // end 'calculate_energy_at_point(edm4hep::TrackPoint&, float)'



    // ------------------------------------------------------------------------
    //! Calculate Sum of Cluster Energies
    // ------------------------------------------------------------------------
    /*! Overloaded function to calculate the sum of energies from a provided
     *  list of edm4eic::Cluster's.
     */
    float calculate_sum_of_energies(std::vector<edm4eic::Cluster>& vecClust) {

      float sum = 0.;
      for (const auto clust : vecClust) {
        sum += clust.getEnergy();
      }
      return sum;

    }  // end 'calculate_sum_of_energies(std::vector<edm4eic::Cluster>&)'



    // ------------------------------------------------------------------------
    //! Calculate Sum of Track Energies at Points
    // ------------------------------------------------------------------------
    /*! Overloaded function to calculate the sum of energies from a provided
     *  list of edm4eic::TrackPoint's.
     */
    float calculate_sum_of_energies(std::vector<edm4eic::TrackPoint>& points, std::optional<float> mass = std::nullopt) {

      float sum = 0.;
      for (const auto point : points) {
	if (mass.has_value()) {
          sum += calculate_energy_at_point(point, mass.value());
	} else {
	  sum += calculate_energy_at_point(point, consts.massPiCharged);
	}
      }
      return sum;

    }  // end 'calculate_sum_of_energies(std::vector<edm4eic::TrackPoint>&, std::optional<float>)'



    // ------------------------------------------------------------------------
    //! Calculate Energy-Weighted Position
    // ------------------------------------------------------------------------
    /*! Calculates the energy-weighted position of a collection of clusters.
     *
     *  TODO it might be handy to let the weighting function be configurable
     *  in a way similar to ClusterRecoCoG...
     */ 
    edm4hep::Vector3f calculate_energy_weighted_centroid(std::vector<edm4eic::Cluster>& clusters) {

      // get sum of energy
      float sum_energy = calculate_sum_of_energies(clusters);;

      edm4hep::Vector3f weighted_position = clusters.at(0).getEnergy() * clusters.at(0).getPosition();
      for (size_t iClust = 1; iClust < clusters.size(); iClust++) {
        weighted_position = weighted_position + (clusters.at(iClust).getEnergy() * clusters.at(iClust).getPosition());
      }
      return weighted_position / sum_energy;

    }  // end 'calculate_energy_weighted_centroid(std::vector<edm4eic::Cluster>&)'



    // ------------------------------------------------------------------------
    //! Calculate Momentum for a Cluster
    // ------------------------------------------------------------------------
    /*! Calculates momentum for an edm4eic::Cluster relative to a
     *  given vertex.  Can provide a mass as an optional argument
     *  if there is a PID hypothesis associated with the cluster.
     */
    edm4hep::Vector3f calculate_momentum(const edm4eic::Cluster& clust, const edm4hep::Vector3f vertex, const float mass = consts.massPi0) {

      // get displacement vector and magnitudes
      const auto  displace   = clust.getPosition() - vertex;
      const float rMagnitude = edm4hep::utils::magnitude(displace);
      const float pMagnitude = (clust.getEnergy() >= mass) ? std::sqrt((clust.getEnergy() * clust.getEnergy()) - (mass * mass)) : 0.;

      // calculate momentum and return
      const edm4hep::Vector3f momentum = (pMagnitude / rMagnitude) * displace;
      return momentum;

    }  // end 'calculate_momentum(edm4eic::Cluster, edm4hep::Vector3f, float)'



    // ------------------------------------------------------------------------
    //! Calculate Sum of Cluster Momenta
    // ------------------------------------------------------------------------
    /*! Function to calculate the sum of 3-momenta from a provided list of
     *  edm4eic::Cluster's relative to a provided vertex. 
     */
    edm4hep::Vector3f calculate_sum_of_momenta(std::vector<edm4eic::Cluster>& vecClust, const edm4hep::Vector3f vertex, const float mass = consts.massPi0) {

      edm4hep::Vector3f sum = PFTools::calculate_momentum(vecClust.at(0), vertex, mass);
      for (size_t iClust = 1; iClust < vecClust.size(); iClust++) {
        sum = sum + PFTools::calculate_momentum(vecClust.at(iClust), vertex, mass);
      }
      return sum;

    }  // end 'calculate_sum_of_momenta(std::vector<edm4eic::Cluster>&, edm4hep::Vector3f)'



    // ------------------------------------------------------------------------
    //! Create Cluster with Substracted Energy
    // ------------------------------------------------------------------------
    /*! Creates a copy of a cluster but with subtracted energy.
     */
    edm4eic::Cluster make_subtracted_cluster(const edm4eic::Cluster& original, const float eneToSub) {

      // recalculate energy error
      const float eneSub = original.getEnergy() - eneToSub;
      const float perErr = original.getEnergyError() / original.getEnergy();
      const float newErr = perErr * eneSub;

      // create new cluster
      edm4eic::MutableCluster subtracted;
      subtracted.setEnergy( eneSub );
      subtracted.setEnergyError( newErr );
      subtracted.setTime( original.getTime() );
      subtracted.setNhits( original.getNhits() );
      subtracted.setPosition( original.getPosition() );
      subtracted.setPositionError( original.getPositionError() );
      subtracted.addToClusters( original );

      // return non-mutable rep
      return static_cast<edm4eic::Cluster>(subtracted);

    }  // end 'make_subtracted_cluster(edm4eic::Cluster&, float)'



    // ------------------------------------------------------------------------
    //! Get Nearest Track Projection
    // ------------------------------------------------------------------------
    /*! Returns the track projection nearest to the provided position in the
     *  eta-phi plane from the provided set of projections.  If no point is
     *  found, then std::nullopt is returned.
     */
    std::optional<edm4eic::TrackPoint> get_nearest_projection(std::vector<edm4eic::TrackPoint>& projections, const edm4hep::Vector3f& position) {
  
      // instantiate point to return
      std::optional<edm4eic::TrackPoint> nearest = std::nullopt;

      // loop over projections
      float dNearest = std::numeric_limits<float>::max();
      for (const auto project : projections) {
        const float dist = calculate_dist_in_eta_phi(position, project.position);
        if (dist < dNearest) {
          nearest = project;
        }
      }
      return nearest;

    }  // end 'get_nearest_projection(std::vector<>, edm4hep::Vector3f&)



    // ------------------------------------------------------------------------
    //! Find Track Projection at a Specific Surface
    // ------------------------------------------------------------------------
    /*! Returns the corresponding to the track projected to the specified
     *  at the specified surface.  If no point is found, then std::nullopt is
     *  returned. 
     */
    std::optional<edm4eic::TrackPoint> find_point_at_surface(const edm4eic::TrackSegment projection, const uint32_t system, const uint64_t surface) {

      // instantiate optional point
      std::optional<edm4eic::TrackPoint> foundPoint = std::nullopt;

      // select from points comprising track projection
      for (const auto point : projection.getPoints()) {
        const bool isInSystem = (point.system == system);
        const bool isAtSurface = (point.surface == surface);
        if (isInSystem && isAtSurface) {
          foundPoint = point;
          break;
        }
      }  // end point loop
      return foundPoint;
    }  // end 'find_point_at_surface(edm4eic::TrackSegment, uint32_t, uint64_t)'

  }  // end PFTools namespace
}  // end eicrecon namespace
