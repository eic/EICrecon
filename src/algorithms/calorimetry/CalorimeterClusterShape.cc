// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chao Peng, Dhevan Gangadharan, Sebouh Paul, Derek Anderson

#include "CalorimeterClusterShape.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/map.hpp>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <Eigen/Core>
#include <Eigen/Eigenvalues>
#include <Eigen/Householder> // IWYU pragma: keep
#include <Eigen/Jacobi>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <complex>
#include <cstddef>
#include <gsl/pointers>
#include <iterator>
#include <utility>
#include <vector>

#include "algorithms/calorimetry/CalorimeterClusterShapeConfig.h"

namespace eicrecon {

void CalorimeterClusterShape::init() {

  // select weighting method
  std::string ew = m_cfg.energyWeight;

  // make it case-insensitive
  std::ranges::transform(ew, ew.begin(), [](char s) { return std::tolower(s); });
  auto it = m_weightMethods.find(ew);
  if (it == m_weightMethods.end()) {
    error("Cannot find energy weighting method {}, choose one from [{}]", m_cfg.energyWeight,
          boost::algorithm::join(m_weightMethods | boost::adaptors::map_keys, ", "));
  } else {
    m_weightFunc = it->second;
  }

} // end 'init()'

/*! Primary algorithm call: algorithm ingests a collection of clusters
   *  and computes their cluster shape parameters.  Clusters are copied
   *  onto output with computed shape parameters.  If associations are
   *  provided, they are copied to the output.
   *
   *  Parameters calculated:
   *    - radius,
   *    - dispersion (energy weighted radius),
   *    - theta-phi cluster widths (2D)
   *    - x-y-z cluster widths (3D)
   */
void CalorimeterClusterShape::process(const CalorimeterClusterShape::Input& input,
                                      const CalorimeterClusterShape::Output& output) const {

  // grab inputs/outputs
  const auto [in_clusters, in_associations] = input;
  auto [out_clusters, out_associations]     = output;

  // exit if no clusters in collection
  if (in_clusters->empty()) {
    debug("No clusters in input collection.");
    return;
  }

  // loop over input clusters
  for (const auto& in_clust : *in_clusters) {

    // copy input cluster
    edm4eic::MutableCluster out_clust = in_clust.clone();

    // set up base for weights
    double logWeightBase = m_cfg.logWeightBase;
    if (!m_cfg.logWeightBaseCoeffs.empty()) {
      double l      = std::log(out_clust.getEnergy() / m_cfg.logWeightBase_Eref);
      logWeightBase = 0;
      for (std::size_t i = 0; i < m_cfg.logWeightBaseCoeffs.size(); i++) {
        logWeightBase += m_cfg.logWeightBaseCoeffs[i] * pow(l, i);
      }
    }

    // ----------------------------------------------------------------------
    // do shape parameter calculation
    // ----------------------------------------------------------------------
    {

      // create addresses for quantities we'll need later
      float radius     = 0;
      float dispersion = 0;
      float w_sum      = 0;

      // set up matrices/vectors
      Eigen::Matrix2f sum2_2D         = Eigen::Matrix2f::Zero();
      Eigen::Matrix3f sum2_3D         = Eigen::Matrix3f::Zero();
      Eigen::Vector2f sum1_2D         = Eigen::Vector2f::Zero();
      Eigen::Vector3f sum1_3D         = Eigen::Vector3f::Zero();
      Eigen::Vector2cf eigenValues_2D = Eigen::Vector2cf::Zero();
      Eigen::Vector3cf eigenValues_3D = Eigen::Vector3cf::Zero();

      // the axis is the direction of the eigenvalue corresponding to the largest eigenvalue.
      edm4hep::Vector3f axis;
      if (out_clust.getNhits() > 1) {
        for (const auto& hit : out_clust.getHits()) {

          // get weight of hit
          const double eTotal = out_clust.getEnergy() * m_cfg.sampFrac;
          const float w       = m_weightFunc(hit.getEnergy(), eTotal, logWeightBase, 0);

          // theta, phi
          Eigen::Vector2f pos2D(edm4hep::utils::anglePolar(hit.getPosition()),
                                edm4hep::utils::angleAzimuthal(hit.getPosition()));
          // x, y, z
          Eigen::Vector3f pos3D(hit.getPosition().x, hit.getPosition().y, hit.getPosition().z);

          const auto delta = out_clust.getPosition() - hit.getPosition();
          radius += delta * delta;
          dispersion += delta * delta * w;

          // Weighted Sum x*x, x*y, x*z, y*y, etc.
          sum2_2D += w * pos2D * pos2D.transpose();
          sum2_3D += w * pos3D * pos3D.transpose();

          // Weighted Sum x, y, z
          sum1_2D += w * pos2D;
          sum1_3D += w * pos3D;

          w_sum += w;
        } // end hit loop

        radius = sqrt((1. / (out_clust.getNhits() - 1.)) * radius);
        if (w_sum > 0) {
          dispersion = sqrt(dispersion / w_sum);

          // normalize matrices
          sum2_2D /= w_sum;
          sum2_3D /= w_sum;
          sum1_2D /= w_sum;
          sum1_3D /= w_sum;

          // 2D and 3D covariance matrices
          Eigen::Matrix2f cov2 = sum2_2D - sum1_2D * sum1_2D.transpose();
          Eigen::Matrix3f cov3 = sum2_3D - sum1_3D * sum1_3D.transpose();

          // Solve for eigenvalues.  Corresponds to out_cluster's 2nd moments (widths)
          Eigen::EigenSolver<Eigen::Matrix2f> es_2D(
              cov2, false); // set to true for eigenvector calculation
          Eigen::EigenSolver<Eigen::Matrix3f> es_3D(
              cov3, true); // set to true for eigenvector calculation

          // eigenvalues of symmetric real matrix are always real
          eigenValues_2D = es_2D.eigenvalues();
          eigenValues_3D = es_3D.eigenvalues();
          //find the eigenvector corresponding to the largest eigenvalue
          auto eigenvectors      = es_3D.eigenvectors();
          auto max_eigenvalue_it = std::ranges::max_element(
              eigenValues_3D, [](auto a, auto b) { return std::real(a) < std::real(b); });
          auto axis_eigen =
              eigenvectors.col(std::distance(eigenValues_3D.begin(), max_eigenvalue_it));
          axis = {
              axis_eigen(0, 0).real(),
              axis_eigen(1, 0).real(),
              axis_eigen(2, 0).real(),
          };
        } // end if weight sum is nonzero
      } // end if n hits > 1

      // set shape parameters
      out_clust.addToShapeParameters(radius);
      out_clust.addToShapeParameters(dispersion);
      out_clust.addToShapeParameters(eigenValues_2D[0].real()); // 2D theta-phi out_cluster width 1
      out_clust.addToShapeParameters(eigenValues_2D[1].real()); // 2D theta-phi out_cluster width 2
      out_clust.addToShapeParameters(eigenValues_3D[0].real()); // 3D x-y-z out_cluster width 1
      out_clust.addToShapeParameters(eigenValues_3D[1].real()); // 3D x-y-z out_cluster width 2
      out_clust.addToShapeParameters(eigenValues_3D[2].real()); // 3D x-y-z out_cluster width 3

      // check axis orientation
      double dot_product = out_clust.getPosition() * axis;
      if (dot_product < 0) {
        axis = -1 * axis;
      }

      // set intrinsic theta/phi
      if (m_cfg.longitudinalShowerInfoAvailable) {
        out_clust.setIntrinsicTheta(edm4hep::utils::anglePolar(axis));
        out_clust.setIntrinsicPhi(edm4hep::utils::angleAzimuthal(axis));
        // TODO intrinsicDirectionError
      } else {
        out_clust.setIntrinsicTheta(NAN);
        out_clust.setIntrinsicPhi(NAN);
      }
    } // end shape parameter calculation

    out_clusters->push_back(out_clust);
    trace("Completed shape calculation for cluster {}", in_clust.getObjectID().index);

    // ----------------------------------------------------------------------
    // if provided, copy associations
    // ----------------------------------------------------------------------
    if ((in_associations != nullptr) && !in_associations->empty()) {
      for (auto in_assoc : *in_associations) {
        if (in_assoc.getRec() == in_clust) {
          auto mc_par    = in_assoc.getSim();
          auto out_assoc = out_associations->create();
          out_assoc.setRec(out_clust);
          out_assoc.setSim(mc_par);
          out_assoc.setWeight(in_assoc.getWeight());
        }
      } // end input association loop
    }
  } // end input cluster loop
  debug("Completed processing input clusters");

} // end 'process(Input&, Output&)'

} // namespace eicrecon
