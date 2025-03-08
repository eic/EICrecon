// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024 Sylvester Joosten, Chao, Chao Peng, Whitney Armstrong, Dhevan Gangadharan, Derek Anderson

/*
 *  Reconstruct the cluster with Center of Gravity method
 *  Logarithmic weighting is used for mimicking energy deposit in transverse direction
 *
 *  Author: Chao Peng (ANL), 09/27/2020
 */

#include <Evaluator/DD4hepUnits.h>
#include <boost/algorithm/string/join.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/range/adaptor/map.hpp>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4hep/RawCalorimeterHit.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <Eigen/Core>
#include <Eigen/Eigenvalues>
#include <Eigen/Householder> // IWYU pragma: keep
#include <cctype>
#include <complex>
#include <cstddef>
#include <gsl/pointers>
#include <iterator>
#include <limits>
#include <map>
#include <optional>
#include <vector>

#include "CalorimeterClusterRecoCoG.h"
#include "algorithms/calorimetry/CalorimeterClusterRecoCoGConfig.h"

namespace eicrecon {

  using namespace dd4hep;

  void CalorimeterClusterRecoCoG::init() {
    // select weighting method
    std::string ew = m_cfg.energyWeight;
    // make it case-insensitive
    std::transform(ew.begin(), ew.end(), ew.begin(), [](char s) { return std::tolower(s); });
    auto it = weightMethods.find(ew);
    if (it == weightMethods.end()) {
      error("Cannot find energy weighting method {}, choose one from [{}]", m_cfg.energyWeight, boost::algorithm::join(weightMethods | boost::adaptors::map_keys, ", "));
      return;
    }
    weightFunc = it->second;
  }

  void CalorimeterClusterRecoCoG::process(
      const CalorimeterClusterRecoCoG::Input& input,
      const CalorimeterClusterRecoCoG::Output& output) const {
#if EDM4EIC_VERSION_MAJOR >= 7
    const auto [proto, mchitassociations] = input;
#else
    const auto [proto, mchits] = input;
#endif
    auto [clusters, associations] = output;

    for (const auto& pcl : *proto) {
      // skip protoclusters with no hits
      if (pcl.hits_size() == 0) {
        continue;
      }

      auto cl_opt = reconstruct(pcl);
      if (! cl_opt.has_value()) {
        continue;
      }
      auto cl = *std::move(cl_opt);

      debug("{} hits: {} GeV, ({}, {}, {})", cl.getNhits(), cl.getEnergy() / dd4hep::GeV, cl.getPosition().x / dd4hep::mm, cl.getPosition().y / dd4hep::mm, cl.getPosition().z / dd4hep::mm);
      clusters->push_back(cl);

      // If sim hits are available, associate cluster with MCParticle
#if EDM4EIC_VERSION_MAJOR >= 7
      if (mchitassociations->size() == 0) {
        debug("Provided MCRecoCalorimeterHitAssociation collection is empty. No truth associations will be performed.");
        continue;
      } else {
        associate(cl, mchitassociations, associations);
      }
#else
      if (mchits->size() == 0) {
        debug("Provided SimCalorimeterHitCollection is empty. No truth association will be performed.");
        continue;
      } else {
        associate(cl, mchits, associations);
      }
#endif
    }
}

std::optional<edm4eic::MutableCluster> CalorimeterClusterRecoCoG::reconstruct(const edm4eic::ProtoCluster& pcl) const {
  edm4eic::MutableCluster cl;
  cl.setNhits(pcl.hits_size());

  debug("hit size = {}", pcl.hits_size());

  // no hits
  if (pcl.hits_size() == 0) {
    return {};
  }

  // calculate total energy, find the cell with the maximum energy deposit
  float totalE = 0.;
  // Used to optionally constrain the cluster eta to those of the contributing hits
  float minHitEta = std::numeric_limits<float>::max();
  float maxHitEta = std::numeric_limits<float>::min();
  auto time      = 0;
  auto timeError = 0;
  for (unsigned i = 0; i < pcl.getHits().size(); ++i) {
    const auto& hit   = pcl.getHits()[i];
    const auto weight = pcl.getWeights()[i];
    debug("hit energy = {} hit weight: {}", hit.getEnergy(), weight);
    auto energy = hit.getEnergy() * weight;
    totalE += energy;
    time += (hit.getTime() - time) * energy / totalE;
    cl.addToHits(hit);
    cl.addToHitContributions(energy);
    const float eta = edm4hep::utils::eta(hit.getPosition());
    if (eta < minHitEta) {
      minHitEta = eta;
    }
    if (eta > maxHitEta) {
      maxHitEta = eta;
    }
  }
  cl.setEnergy(totalE / m_cfg.sampFrac);
  cl.setEnergyError(0.);
  cl.setTime(time);
  cl.setTimeError(timeError);

  // center of gravity with logarithmic weighting
  float tw = 0.;
  auto v   = cl.getPosition();

  double logWeightBase=m_cfg.logWeightBase;
  if (m_cfg.logWeightBaseCoeffs.size() != 0){
    double l=log(cl.getEnergy()/m_cfg.logWeightBase_Eref);
    logWeightBase=0;
    for(std::size_t i =0; i<m_cfg.logWeightBaseCoeffs.size(); i++){
      logWeightBase += m_cfg.logWeightBaseCoeffs[i]*pow(l,i);
    }
  }

  for (unsigned i = 0; i < pcl.getHits().size(); ++i) {
    const auto& hit   = pcl.getHits()[i];
    const auto weight = pcl.getWeights()[i];
    //      _DBG_<<" -- weight = " << weight << "  E=" << hit.getEnergy() << " totalE=" <<totalE << " log(E/totalE)=" << std::log(hit.getEnergy()/totalE) << std::endl;
    float w           = weightFunc(hit.getEnergy() * weight, totalE, logWeightBase, 0);
    tw += w;
    v = v + (hit.getPosition() * w);
  }
  if (tw == 0.) {
    warning("zero total weights encountered, you may want to adjust your weighting parameter.");
    return {};
  }
  cl.setPosition(v / tw);
  cl.setPositionError({}); // @TODO: Covariance matrix

  // Optionally constrain the cluster to the hit eta values
  if (m_cfg.enableEtaBounds) {
    const bool overflow  = (edm4hep::utils::eta(cl.getPosition()) > maxHitEta);
    const bool underflow = (edm4hep::utils::eta(cl.getPosition()) < minHitEta);
    if (overflow || underflow) {
      const double newEta   = overflow ? maxHitEta : minHitEta;
      const double newTheta = edm4hep::utils::etaToAngle(newEta);
      const double newR     = edm4hep::utils::magnitude(cl.getPosition());
      const double newPhi   = edm4hep::utils::angleAzimuthal(cl.getPosition());
      cl.setPosition(edm4hep::utils::sphericalToVector(newR, newTheta, newPhi));
      debug("Bound cluster position to contributing hits due to {}", (overflow ? "overflow" : "underflow"));
    }
  }

  // Additional convenience variables

  //_______________________________________
  // Calculate cluster profile:
  //    radius,
  //    dispersion (energy weighted radius),
  //    theta-phi cluster widths (2D)
  //    x-y-z cluster widths (3D)
  float radius = 0, dispersion = 0, w_sum = 0;

  Eigen::Matrix2f sum2_2D = Eigen::Matrix2f::Zero();
  Eigen::Matrix3f sum2_3D = Eigen::Matrix3f::Zero();
  Eigen::Vector2f sum1_2D = Eigen::Vector2f::Zero();
  Eigen::Vector3f sum1_3D = Eigen::Vector3f::Zero();
  Eigen::Vector2cf eigenValues_2D = Eigen::Vector2cf::Zero();
  Eigen::Vector3cf eigenValues_3D = Eigen::Vector3cf::Zero();
  // the axis is the direction of the eigenvalue corresponding to the largest eigenvalue.
  edm4hep::Vector3f axis;

  if (cl.getNhits() > 1) {
    for (const auto& hit : pcl.getHits()) {
      float w = weightFunc(hit.getEnergy(), totalE, logWeightBase, 0);

      // theta, phi
      Eigen::Vector2f pos2D( edm4hep::utils::anglePolar( hit.getPosition() ), edm4hep::utils::angleAzimuthal( hit.getPosition() ) );
      // x, y, z
      Eigen::Vector3f pos3D( hit.getPosition().x, hit.getPosition().y, hit.getPosition().z );

      const auto delta = cl.getPosition() - hit.getPosition();
      radius          += delta * delta;
      dispersion      += delta * delta * w;

      // Weighted Sum x*x, x*y, x*z, y*y, etc.
      sum2_2D += w * pos2D * pos2D.transpose();
      sum2_3D += w * pos3D * pos3D.transpose();

      // Weighted Sum x, y, z
      sum1_2D += w * pos2D;
      sum1_3D += w * pos3D;

      w_sum += w;
    }

    radius     = sqrt((1. / (cl.getNhits() - 1.)) * radius);
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

      // Solve for eigenvalues.  Corresponds to cluster's 2nd moments (widths)
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
    }
  }

  cl.addToShapeParameters( radius );
  cl.addToShapeParameters( dispersion );
  cl.addToShapeParameters( eigenValues_2D[0].real() ); // 2D theta-phi cluster width 1
  cl.addToShapeParameters( eigenValues_2D[1].real() ); // 2D theta-phi cluster width 2
  cl.addToShapeParameters( eigenValues_3D[0].real() ); // 3D x-y-z cluster width 1
  cl.addToShapeParameters( eigenValues_3D[1].real() ); // 3D x-y-z cluster width 2
  cl.addToShapeParameters( eigenValues_3D[2].real() ); // 3D x-y-z cluster width 3


  double dot_product = cl.getPosition() * axis;
  if (dot_product < 0) {
    axis = -1 * axis;
  }

  if (m_cfg.longitudinalShowerInfoAvailable) {
    cl.setIntrinsicTheta(edm4hep::utils::anglePolar(axis));
    cl.setIntrinsicPhi(edm4hep::utils::angleAzimuthal(axis));
    // TODO intrinsicDirectionError
  } else {
    cl.setIntrinsicTheta(NAN);
    cl.setIntrinsicPhi(NAN);
  }

  return std::move(cl);
}

void CalorimeterClusterRecoCoG::associate(
  const edm4eic::Cluster& cl,
#if EDM4EIC_VERSION_MAJOR >= 7
  const edm4eic::MCRecoCalorimeterHitAssociationCollection* mchitassociations,
#else
  const edm4hep::SimCalorimeterHitCollection* mchits,
#endif
  edm4eic::MCRecoClusterParticleAssociationCollection* assocs
) const {
  // --------------------------------------------------------------------------
  // Association Logic
  // --------------------------------------------------------------------------
  /*  1. identify all sim hits associated with a given protocluster, and sum
   *     the energy of the sim hits.
   *  2. for each sim hit
   *     - identify parents of each contributing particles; and
   *     - if parent is a primary particle, add to list of contributors
   *       and sum the energy contributed by the parent.
   *  3. create an association for each contributing primary with a weight
   *     of contributed energy over total sim hit energy.
   */

  // lambda to compare MCParticles
  auto compare = [](const edm4hep::MCParticle& lhs, const edm4hep::MCParticle& rhs) {
    if (lhs.getObjectID().collectionID == rhs.getObjectID().collectionID) {
      return (lhs.getObjectID().index < rhs.getObjectID().index);
    } else {
      return (lhs.getObjectID().collectionID < rhs.getObjectID().collectionID);
    }
  };

  // bookkeeping maps for associated primaries
  std::map<edm4hep::MCParticle, double, decltype(compare)> mapMCParToContrib(compare);

  // --------------------------------------------------------------------------
  // 1. get associated sim hits and sum energy
  // --------------------------------------------------------------------------
  double eSimHitSum = 0.;
  for (auto clhit : cl.getHits()) {
    // vector to hold associated sim hits
    std::vector<edm4hep::SimCalorimeterHit> vecAssocSimHits;

#if EDM4EIC_VERSION_MAJOR >= 7
    for (const auto& hitAssoc : *mchitassociations) {
      // if found corresponding raw hit, add sim hit to vector
      // and increment energy sum
      if (clhit.getRawHit() == hitAssoc.getRawHit()) {
        vecAssocSimHits.push_back(hitAssoc.getSimHit());
        eSimHitSum += vecAssocSimHits.back().getEnergy();
      }

    }
#else
    for (const auto& mchit : *mchits) {
      if (mchit.getCellID() == clhit.getCellID()) {
         vecAssocSimHits.push_back(mchit);
        break;
      }
    }

    // if no matching cell ID found, continue
    // otherwise increment sum
    if (vecAssocSimHits.empty()) {
      debug("No matching SimHit for hit {}", clhit.getCellID());
      continue;
    } else {
      eSimHitSum += vecAssocSimHits.back().getEnergy();
    }
#endif
    debug("{} associated sim hits found for reco hit (cell ID = {})", vecAssocSimHits.size(), clhit.getCellID());

    // ------------------------------------------------------------------------
    // 2. loop through associated sim hits
    // ------------------------------------------------------------------------
    for (const auto& simHit : vecAssocSimHits) {
      for (const auto& contrib : simHit.getContributions()) {
        // --------------------------------------------------------------------
        // grab primary responsible for contribution & increment relevant sum
        // --------------------------------------------------------------------
        edm4hep::MCParticle primary = get_primary(contrib);
        mapMCParToContrib[primary] += contrib.getEnergy();

        trace("Identified primary: id = {}, pid = {}, total energy = {}, contributed = {}",
          primary.getObjectID().index,
          primary.getPDG(),
          primary.getEnergy(),
          mapMCParToContrib[primary]
        );
      }
    }
  }
  debug("Found {} primaries contributing a total of {} GeV", mapMCParToContrib.size(), eSimHitSum);

  // --------------------------------------------------------------------------
  // 3. create association for each contributing primary
  // --------------------------------------------------------------------------
  for (auto [part, contribution] : mapMCParToContrib) {
    // calculate weight
    const double weight = contribution / eSimHitSum;

    // set association
    auto assoc = assocs->create();
    assoc.setRecID(cl.getObjectID().index); // if not using collection, this is always set to -1
    assoc.setSimID(part.getObjectID().index);
    assoc.setWeight(weight);
    assoc.setRec(cl);
    assoc.setSim(part);
    debug("Associated cluster #{} to MC Particle #{} (pid = {}, status = {}, energy = {}) with weight ({})",
      cl.getObjectID().index,
      part.getObjectID().index,
      part.getPDG(),
      part.getGeneratorStatus(),
      part.getEnergy(),
      weight
    );
  }
}

edm4hep::MCParticle CalorimeterClusterRecoCoG::get_primary(const edm4hep::CaloHitContribution& contrib) const {
  // get contributing particle
  const auto contributor = contrib.getParticle();

  // walk back through parents to find primary
  //   - TODO finalize primary selection. This
  //     can be improved!!
  edm4hep::MCParticle primary = contributor;
  while (primary.parents_size() > 0) {
    if (primary.getGeneratorStatus() != 0) break;
    primary = primary.getParents(0);
  }
  return primary;
}

}
