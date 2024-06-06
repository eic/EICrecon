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

    const auto [proto, mchits] = input;
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
      if (mchits->size() == 0) {
        debug("No mcHitCollection was provided, so no truth association will be performed.");
        continue;
      }

      auto assoc_opt = associate(cl, mchits);
      if (!assoc_opt.has_value()) {
        continue;
      }
      auto assoc = *std::move(assoc_opt);
      associations->push_back(assoc);

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
  auto time       = pcl.getHits()[0].getTime();
  auto timeError  = pcl.getHits()[0].getTimeError();
  for (unsigned i = 0; i < pcl.getHits().size(); ++i) {
    const auto& hit   = pcl.getHits()[i];
    const auto weight = pcl.getWeights()[i];
    debug("hit energy = {} hit weight: {}", hit.getEnergy(), weight);
    auto energy = hit.getEnergy() * weight;
    totalE += energy;
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

  // best estimate on the cluster direction is the cluster position
  // for simple 2D CoG clustering
  cl.setIntrinsicTheta(edm4hep::utils::anglePolar(cl.getPosition()));
  cl.setIntrinsicPhi(edm4hep::utils::angleAzimuthal(cl.getPosition()));
  // TODO errors

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
  //the axis is the direction of the eigenvalue corresponding to the largest eigenvalue.
  double axis_x=0, axis_y=0, axis_z=0;
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
      auto axis = eigenvectors.col(std::distance(
            eigenValues_3D.begin(),
            max_eigenvalue_it
        ));
      axis_x=axis(0,0).real();
      axis_y=axis(1,0).real();
      axis_z=axis(2,0).real();
      double norm=sqrt(axis_x*axis_x+axis_y*axis_y+axis_z*axis_z);
      if (norm!=0){
        axis_x/=norm;
        axis_y/=norm;
        axis_z/=norm;
      }
    }
  }

  cl.addToShapeParameters( radius );
  cl.addToShapeParameters( dispersion );
  cl.addToShapeParameters( eigenValues_2D[0].real() ); // 2D theta-phi cluster width 1
  cl.addToShapeParameters( eigenValues_2D[1].real() ); // 2D theta-phi cluster width 2
  cl.addToShapeParameters( eigenValues_3D[0].real() ); // 3D x-y-z cluster width 1
  cl.addToShapeParameters( eigenValues_3D[1].real() ); // 3D x-y-z cluster width 2
  cl.addToShapeParameters( eigenValues_3D[2].real() ); // 3D x-y-z cluster width 3
  //last 3 shape parameters are the components of the axis direction
  cl.addToShapeParameters( axis_x );
  cl.addToShapeParameters( axis_y );
  cl.addToShapeParameters( axis_z );
  return std::move(cl);
}

//------------------------------------------------------------------------
std::optional<edm4eic::MutableMCRecoClusterParticleAssociation> CalorimeterClusterRecoCoG::associate(
  const edm4eic::Cluster& cl,
  const edm4hep::SimCalorimeterHitCollection* mchits
) const {

  // 1. idenitfy sim hits associated w/ protocluster and sum their energy
  // 2. sort list of associated mcHits in decreasing energy
  // 3. walk through contributions to find MCParticle which contributed
  //    the most energy
  // 4. associate cluster to that MCParticle
  edm4eic::MutableMCRecoClusterParticleAssociation assoc;

  // make sure book-keeping containers are empty
  m_vecSimHitIndexVsEne.clear();
  m_mapMCIndexToContrib.clear();

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

    // add index to list
    m_vecSimHitIndexVsEne.emplace_back(
      std::make_pair(iSimMatch, (*mchits)[iSimMatch].getEnergy())
    );
    ++iHit;
  }
  debug("Sum of energy in sim hits = {}", eSimHitSum);

  // 2. sort sim hits in decreasing energy
  std::sort(
    m_vecSimHitIndexVsEne.begin(),
    m_vecSimHitIndexVsEne.end(),
    [](const auto& lhs, const auto& rhs) {
      return (lhs.second > rhs.second);
    }
  );
  trace("Sorted sim hit energies.");

  // 3. find biggest contributing MCParticle
  bool   foundAssoc = false;
  double eConChecked = 0.;
  for (std::size_t iSimVsEne = 0; iSimVsEne < m_vecSimHitIndexVsEne.size(); ++iSimVsEne) {

    const std::size_t iSim = m_vecSimHitIndexVsEne[iSimVsEne].first;
    const auto mchit = (*mchits)[iSim];
    for (std::size_t iContrib = 0; const auto& contrib : mchit.getContributions()) {

      // get particle
      const auto par = contrib.getParticle();

      // get index in MCParticles & contribution
      const int index = par.getObjectID().index;
      const double eContrib = contrib.getEnergy();

      // increment sums accordingly
      if (m_mapMCIndexToContrib.find(index) == m_mapMCIndexToContrib.end()) {
        m_mapMCIndexToContrib.insert({
          index,
          std::make_pair(iContrib, eContrib)
        });
      } else {
        m_mapMCIndexToContrib[index].second += eContrib;
      }
      eConChecked += eContrib;
    }  // end contrib loop

    // grab current max
    const auto maxContrib = std::max_element(
      m_mapMCIndexToContrib.begin(),
      m_mapMCIndexToContrib.end(),
      [](const auto& lhs, const auto& rhs) {
        return lhs.second.second < rhs.second.second;
      }
    );

    // 4. if max is more than remaining energy to check or
    //    at last sim hit, set association and break
    if (
      (maxContrib->second.second > (eSimHitSum - eConChecked)) ||
      (iSimVsEne == (m_vecSimHitIndexVsEne.size() - 1))
    ) {

      // grab corresponding particle and print debugging messages
      auto mcp = (*mchits)[iSim].getContributions(maxContrib->second.first).getParticle();
      debug("corresponding mc hit energy {} at index {}", (*mchits)[iSim].getEnergy(), (*mchits)[iSim].getObjectID().index);
      debug("from MCParticle index {}, PDG {}, {}", mcp.getObjectID().index, mcp.getPDG(), edm4hep::utils::magnitude(mcp.getMomentum()));

      // set association
      assoc.setRecID(cl.getObjectID().index); // if not using collection, this is always set to -1
      assoc.setSimID(mcp.getObjectID().index);
      assoc.setWeight(1.0);
      assoc.setRec(cl);
      assoc.setSim(mcp);
      break;
    }
  }  // end hit loop
  return std::move(assoc);

}  // end 'associate(edm4eic::Cluster&, edm4hep::SimCalorimeterHit*)'

} // eicrecon
