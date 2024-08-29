// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten, Chao, Chao Peng, Whitney Armstrong, Dhevan Gangadharan

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
#include <edm4hep/CaloHitContributionCollection.h>
#include <edm4hep/MCParticleCollection.h>
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
        debug("No mcHitAssociationCollection was provided, so no truth association will be performed.");
#else
      if (mchits->size() == 0) {
        debug("No mcHitCollection was provided, so no truth association will be performed.");
#endif
        continue;
      } else {
#if EDM4EIC_VERSION_MAJOR >= 7
        associate(cl, mchitassociations, associations);
#else
        associate(cl, mchits, associations);
#endif
      }

    }  // end protocluster loop
}

//------------------------------------------------------------------------
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

//------------------------------------------------------------------------
void CalorimeterClusterRecoCoG::associate(
  const edm4eic::Cluster& cl,
#if EDM4EIC_VERSION_MAJOR >= 7
  const edm4eic::MCRecoCalorimeterHitAssociationCollection* mchitassociations,
#else
  const edm4hep::SimCalorimeterHitCollection* mchits,
#endif
  edm4eic::MCRecoClusterParticleAssociationCollection* assocs
) const {

  // 1. idenitfy sim hits associated w/ protocluster and sum their energy
  // 2. for each sim hit, identify contributing primaries and sum their contributed energy
  // 3. create an association for each contributiong primary with a weight of contributed
  //    over total energy

  // bookkeeping containers
  std::map<int, std::pair<int, int>> m_mapMCParToSimIndices;
  std::map<int, double> m_mapMCParToContrib;

  // 1. get associated sim hits and sum energy
  double eSimHitSum = 0.;
  for (std::size_t iHit = 0; auto clhit : cl.getHits()) {

    // grab corresponding sim hit and increment sum
    //   - FIXME use RecHit-RawHit associations when ready
    std::size_t iSimMatch = mchits->size();
    for (std::size_t iSim = 0; iSim < mchits->size(); ++iSim) {
      if ((*mchits)[iSim].getCellID() == clhit.getCellID()) {
        iSimMatch = iSim;
        break;
      }
    }  // end sim hit loop

    if (iSimMatch == mchits->size()) {
      debug("No matching SimHit for hit {}", clhit.getCellID());
      continue;
    }
    eSimHitSum += (*mchits)[iSimMatch].getEnergy();

    // 2. walk back through contributions to find primaries
    for (std::size_t iContrib = 0; const auto& contrib : (*mchits)[iSimMatch].getContributions()) {

      // grab primary responsible for contribution
      edm4hep::MCParticle primary;
      get_primary(contrib, primary);

      // increment sums accordingly
      const int idPrim = primary.getObjectID().index;
      if (m_mapMCParToContrib.find(idPrim) == m_mapMCParToContrib.end()) {
        m_mapMCParToSimIndices.insert(
          {idPrim, std::make_pair(iSimMatch, iContrib)}
        );
        m_mapMCParToContrib.insert(
          {idPrim, contrib.getEnergy()}
        );
      } else {
        m_mapMCParToContrib[idPrim] += contrib.getEnergy();
      }
      trace("Identified primary: id = {}, pid = {}, total energy = {}, contributed = {}",
        primary.getObjectID().index,
        primary.getPDG(),
        primary.getEnergy(),
        m_mapMCParToContrib[idPrim]
      );

    }  // end contribution loop
  }  // end hit loop
  debug("Found {} primaries contributing a total of {} GeV", m_mapMCParToContrib.size(), eSimHitSum);

  // 3. create association for each contributing primary
  for (const auto& parAndSimIndices : m_mapMCParToSimIndices) {

    // grab indices, calculate weight
    const uint32_t iSimHit = parAndSimIndices.second.first;
    const uint32_t iContrib = parAndSimIndices.second.second;
    const double weight = m_mapMCParToContrib[parAndSimIndices.first] / eSimHitSum;

    // get relevant MCParticle
    edm4hep::MCParticle primary;
    get_primary((*mchits)[iSimHit].getContributions(iContrib), primary);

    // set association
    auto assoc = assocs->create();
    assoc.setRecID(cl.getObjectID().index); // if not using collection, this is always set to -1
    assoc.setSimID(primary.getObjectID().index);
    assoc.setWeight(weight);
    assoc.setRec(cl);
    assoc.setSim(primary);
    debug("Associated cluster #{} to MC Particle #{} (pid = {}, status = {}, energy = {}) with weight ()",
      cl.getObjectID().index,
      primary.getObjectID().index,
      primary.getPDG(),
      primary.getGeneratorStatus(),
      primary.getEnergy(),
      weight
    );

  }  // end contributor loop
  return;

}  // end 'associate(edm4eic::Cluster&, edm4eic::MCRecoCalorimeterHitAssocationCollection OR edm4hep::SimCalorimeterHitCollection*, edm4eic::MCRecoClusterParticleAssociationCollection*)'

//------------------------------------------------------------------------
void CalorimeterClusterRecoCoG::get_primary(
  const edm4hep::CaloHitContribution& contrib,
  edm4hep::MCParticle& primary
) const {

  // get contributing particle
  const auto contributor = contrib.getParticle();

  // walk back through parents to find primary
  //   - TODO finalize primary selection. This
  //     can be improved!!
  primary = contributor;
  while (primary.parents_size() > 0) {
    primary = primary.getParents(0);
    if (primary.getGeneratorStatus() == 1) break;
  }
  return;

}  // end 'get_primary(edm4hep::CaloHitContribution&, edm4hep::MCParticle&)'

} // eicrecon
