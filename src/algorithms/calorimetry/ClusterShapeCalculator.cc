// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 <OG PEOPLE>, Derek Anderson

#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <Eigen/Core>
#include <Eigen/Eigenvalues>
#include <Eigen/Householder>
#include <cmath>

// algorithm definition
#include "ClusterShapeCalculator.h"



namespace eicrecon {

  // --------------------------------------------------------------------------
  //! Initialize algorithm
  // --------------------------------------------------------------------------
  void ClusterShapeCalculator::init() {

    //... nothing to do ...//

  }  // end 'init(dd4hep::Detector*)'



  // --------------------------------------------------------------------------
  //! Process inputs
  // --------------------------------------------------------------------------
  /*! Primary algorithm call: algorithm ingests a collection of clusters
   *  and computes their cluster shape parameters.  Clusters are copied
   *  onto output with computed shape parameters.  If associations are
   *  provided, they are copied to the output.
   */
  void ClusterShapeCalculator::process(
    const ClusterShapeCalculator::Input& input,
    const ClusterShapeCalculator::Output& output
  ) const {

    // grab inputs/outputs
    const auto [in_clusters, in_associations] = input;
    auto [out_clusters, out_associations] = output;

    // exit if no clusters in collection
    if (in_clusters->size() == 0) {
      debug("No clusters in input collection.");
      return;
    }

    // loop over input clusters
    for (const auto& in_clust : *in_clusters) {

      // copy input cluster
      edm4eic::MutableCluster out_clust = in_clust.clone();

      // ----------------------------------------------------------------------
      // do shape parameter calculation
      // ----------------------------------------------------------------------
      {

        // create addresses for quantities we'll need later
        float radius = 0, dispersion = 0, w_sum = 0;

        // set up matrices/vectors
        Eigen::Matrix2f sum2_2D = Eigen::Matrix2f::Zero();
        Eigen::Matrix3f sum2_3D = Eigen::Matrix3f::Zero();
        Eigen::Vector2f sum1_2D = Eigen::Vector2f::Zero();
        Eigen::Vector3f sum1_3D = Eigen::Vector3f::Zero();
        Eigen::Vector2cf eigenValues_2D = Eigen::Vector2cf::Zero();
        Eigen::Vector3cf eigenValues_3D = Eigen::Vector3cf::Zero();

        // the axis is the direction of the eigenvalue corresponding to the largest eigenvalue.
        edm4hep::Vector3f axis;
        if (out_clust.getNhits() > 1) {
          for (std::size_t iHit = 0; const auto& hit : out_clust.getHits()) {

            // get weight of hit
            const float w = out_clust.getHitContributions()[iHit] / hit.getEnergy();

            // theta, phi
            Eigen::Vector2f pos2D( edm4hep::utils::anglePolar( hit.getPosition() ), edm4hep::utils::angleAzimuthal( hit.getPosition() ) );
            // x, y, z
            Eigen::Vector3f pos3D( hit.getPosition().x, hit.getPosition().y, hit.getPosition().z );

            const auto delta = out_clust.getPosition() - hit.getPosition();
            radius     += delta * delta;
            dispersion += delta * delta * w;

            // Weighted Sum x*x, x*y, x*z, y*y, etc.
            sum2_2D += w * pos2D * pos2D.transpose();
            sum2_3D += w * pos3D * pos3D.transpose();

            // Weighted Sum x, y, z
            sum1_2D += w * pos2D;
            sum1_3D += w * pos3D;

            w_sum += w;
            ++iHit;
          }  // end hit loop

          radius = sqrt((1. / (out_clust.getNhits() - 1.)) * radius);
          if( w_sum > 0 ) {
            dispersion = sqrt( dispersion / w_sum );

            // normalize matrices
            sum2_2D /= w_sum;
            sum2_3D /= w_sum;
            sum1_2D /= w_sum;
            sum1_3D /= w_sum;

            // 2D and 3D covariance matrices
            Eigen::Matrix2f cov2 = sum2_2D - sum1_2D * sum1_2D.transpose();
            Eigen::Matrix3f cov3 = sum2_3D - sum1_3D * sum1_3D.transpose();

            // Solve for eigenvalues.  Corresponds to out_cluster's 2nd moments (widths)
            Eigen::EigenSolver<Eigen::Matrix2f> es_2D(cov2, false); // set to true for eigenvector calculation
            Eigen::EigenSolver<Eigen::Matrix3f> es_3D(cov3, true); // set to true for eigenvector calculation

            // eigenvalues of symmetric real matrix are always real
            eigenValues_2D = es_2D.eigenvalues();
            eigenValues_3D = es_3D.eigenvalues();
            //find the eigenvector corresponding to the largest eigenvalue
            auto eigenvectors= es_3D.eigenvectors();
            auto max_eigenvalue_it = std::max_element(
              eigenValues_3D.begin(),
              eigenValues_3D.end(),
              [](auto a, auto b) {
                  return std::real(a) < std::real(b);
              }
            );
            auto axis_eigen = eigenvectors.col(std::distance(
                  eigenValues_3D.begin(),
                  max_eigenvalue_it
              ));
            axis = {
              axis_eigen(0,0).real(),
              axis_eigen(1,0).real(),
              axis_eigen(2,0).real(),
            };
          }  // end if weight sum is nonzero
        }  // end if n hits > 1

        // set shape parameters
        out_clust.addToShapeParameters( radius );
        out_clust.addToShapeParameters( dispersion );
        out_clust.addToShapeParameters( eigenValues_2D[0].real() ); // 2D theta-phi out_cluster width 1
        out_clust.addToShapeParameters( eigenValues_2D[1].real() ); // 2D theta-phi out_cluster width 2
        out_clust.addToShapeParameters( eigenValues_3D[0].real() ); // 3D x-y-z out_cluster width 1
        out_clust.addToShapeParameters( eigenValues_3D[1].real() ); // 3D x-y-z out_cluster width 2
        out_clust.addToShapeParameters( eigenValues_3D[2].real() ); // 3D x-y-z out_cluster width 3
      }  // end shape parameter calculation

      // ----------------------------------------------------------------------
      // if provided, copy associations
      // ----------------------------------------------------------------------
      for (auto in_assoc : *in_associations) {
        if (in_assoc.getRec() == in_clust) {
          auto mc_par = in_assoc.getSim();
          auto out_assoc = out_associations->create();
          out_assoc.setRecID( out_clust.getObjectID().index );
          out_assoc.setSimID( mc_par.getObjectID().index );
          out_assoc.setRec( out_clust );
          out_assoc.setSim( mc_par );
          out_assoc.setWeight( in_assoc.getWeight() );
        }
      }  // end input association loop
    }  // end input cluster loop
  }  // end 'process(Input&, Output&)'

}  // end eicrecon namespace
