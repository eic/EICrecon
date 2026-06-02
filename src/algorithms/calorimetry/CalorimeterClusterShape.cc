// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chao Peng, Dhevan Gangadharan, Sebouh Paul, Derek Anderson

#include "CalorimeterClusterShape.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/map.hpp>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4hep/MCParticle.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <podio/detail/Link.h>
#include <podio/detail/LinkCollectionImpl.h>
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
#include <memory>
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
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  auto [out_clusters, out_links, out_associations] = output;
#else
  auto [out_clusters, out_associations] = output;
#endif

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
      double w_sum     = 0;

      // set up matrices/vectors
      // 2D (theta-phi) uses float — angles are O(1), no precision issue.
      // 3D (x-y-z) uses double — hit positions are O(1000) mm; float32 causes
      //   catastrophic cancellation in cov[2,2] = <z²> - <z>² for detectors
      //   with fixed large z (e.g. FEMC at z~3500 mm), giving spurious sigma3.
      Eigen::Matrix2f sum2_2D         = Eigen::Matrix2f::Zero();
      Eigen::Matrix3d sum2_3D         = Eigen::Matrix3d::Zero();
      Eigen::Vector2f sum1_2D         = Eigen::Vector2f::Zero();
      Eigen::Vector3d sum1_3D         = Eigen::Vector3d::Zero();
      Eigen::Vector2cf eigenValues_2D = Eigen::Vector2cf::Zero();
      Eigen::Vector3d eigenValues_3D  = Eigen::Vector3d::Zero();

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
          // x, y, z — double to avoid float32 cancellation for large z
          Eigen::Vector3d pos3D(hit.getPosition().x, hit.getPosition().y, hit.getPosition().z);

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
          sum2_2D /= static_cast<float>(w_sum);
          sum2_3D /= w_sum;
          sum1_2D /= static_cast<float>(w_sum);
          sum1_3D /= w_sum;

          // 2D and 3D covariance matrices
          Eigen::Matrix2f cov2 = sum2_2D - sum1_2D * sum1_2D.transpose();
          Eigen::Matrix3d cov3 = sum2_3D - sum1_3D * sum1_3D.transpose();

          // Use SelfAdjointEigenSolver for symmetric covariance matrices.
          // More accurate than EigenSolver for symmetric matrices, guarantees
          // real eigenvalues, and returns them sorted ascending: [0]=smallest.
          Eigen::SelfAdjointEigenSolver<Eigen::Matrix2f> es_2D(cov2);
          Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es_3D(cov3);

          // eigenvalues of symmetric real matrix are always real
          // Store descending: [0]=largest, [2]=smallest
          auto ev2          = es_2D.eigenvalues(); // ascending real float
          eigenValues_2D[0] = ev2[1];              // largest
          eigenValues_2D[1] = ev2[0];              // smallest

          auto ev3          = es_3D.eigenvalues(); // ascending real double
          eigenValues_3D[0] = ev3[2];              // largest
          eigenValues_3D[1] = ev3[1];
          eigenValues_3D[2] = ev3[0]; // smallest (0 for flat-z detectors)

          // eigenvector for largest eigenvalue (index 2 in ascending order)
          auto axis_eigen = es_3D.eigenvectors().col(2);
          axis            = {
              static_cast<float>(axis_eigen(0)),
              static_cast<float>(axis_eigen(1)),
              static_cast<float>(axis_eigen(2)),
          };
        } // end if weight sum is nonzero
      } // end if n hits > 1

      // set shape parameters
      out_clust.addToShapeParameters(radius);
      out_clust.addToShapeParameters(dispersion);
      out_clust.addToShapeParameters(
          std::sqrt(std::abs(eigenValues_2D[0].real()))); // 2D theta-phi out_cluster width 1 [rad]
      out_clust.addToShapeParameters(
          std::sqrt(std::abs(eigenValues_2D[1].real()))); // 2D theta-phi out_cluster width 2 [rad]
      out_clust.addToShapeParameters(
          std::sqrt(std::abs(eigenValues_3D[0]))); // 3D x-y-z out_cluster width 1 [mm]
      out_clust.addToShapeParameters(
          std::sqrt(std::abs(eigenValues_3D[1]))); // 3D x-y-z out_cluster width 2 [mm]
      out_clust.addToShapeParameters(
          std::sqrt(std::abs(eigenValues_3D[2]))); // 3D x-y-z out_cluster width 3 [mm]

      // check axis orientation
      double dot_product = out_clust.getPosition() * axis;
      if (dot_product < 0) {
        axis = -1 * axis;
      }

      // set intrinsic theta/phi from 3D principal axis
      float intrinsicTheta = edm4hep::utils::anglePolar(axis);
      float intrinsicPhi   = edm4hep::utils::angleAzimuthal(axis);
      out_clust.setIntrinsicTheta(intrinsicTheta);
      out_clust.setIntrinsicPhi(intrinsicPhi);
      // TODO intrinsicDirectionError

      trace("ClusterShape: radius={:.3f} mm dispersion={:.3f} mm "
            "2D_w1={:.4f} rad 2D_w2={:.4f} rad "
            "3D_w1={:.3f} mm 3D_w2={:.3f} mm 3D_w3={:.3f} mm "
            "intrinsicTheta={:.4f} intrinsicPhi={:.4f}",
            radius, dispersion, std::sqrt(std::abs(eigenValues_2D[0].real())),
            std::sqrt(std::abs(eigenValues_2D[1].real())), std::sqrt(std::abs(eigenValues_3D[0])),
            std::sqrt(std::abs(eigenValues_3D[1])), std::sqrt(std::abs(eigenValues_3D[2])),
            intrinsicTheta, intrinsicPhi);
    } // end shape parameter calculation

    out_clusters->push_back(out_clust);
    trace("Completed shape calculation for cluster {}", in_clust.getObjectID().index);

    // ----------------------------------------------------------------------
    // if provided, copy associations
    // ----------------------------------------------------------------------
    for (auto in_assoc : *in_associations) {
      if (in_assoc.getRec() == in_clust) {
        auto mc_par = in_assoc.getSim();
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
        auto out_link = out_links->create();
        out_link.setFrom(out_clust);
        out_link.setTo(mc_par);
        out_link.setWeight(in_assoc.getWeight());
#endif
        auto out_assoc = out_associations->create();
        out_assoc.setRec(out_clust);
        out_assoc.setSim(mc_par);
        out_assoc.setWeight(in_assoc.getWeight());
      }
    } // end input association loop
  } // end input cluster loop
  debug("Completed processing input clusters");

} // end 'process(Input&, Output&)'

} // namespace eicrecon
