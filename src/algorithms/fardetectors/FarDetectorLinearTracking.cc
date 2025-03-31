// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2025, Simon Gardner

#include <DD4hep/VolumeManager.h>
#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <algorithms/geo.h>
#include <edm4hep/MCParticle.h>
#include <edm4eic/Cov2f.h>
#include <edm4eic/Cov3f.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/MCRecoTrackParticleAssociationCollection.h>
#include <edm4eic/vector_utils.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <stdint.h>
#include <Eigen/Geometry>
#include <Eigen/Householder>
#include <Eigen/Jacobi>
#include <Eigen/QR>
#include <Eigen/SVD>
#include <algorithm>
#include <cmath>
#include <utility>

#include "FarDetectorLinearTracking.h"
#include "algorithms/fardetectors/FarDetectorLinearTrackingConfig.h"

namespace eicrecon {

    void FarDetectorLinearTracking::init() {

      // For changing how strongly each layer hit is in contributing to the fit
      m_layerWeights = Eigen::VectorXd::Constant(m_cfg.n_layer,1);

      // For checking the direction of the track from theta and phi angles
      m_optimumDirection = Eigen::Vector3d::UnitZ();
      m_optimumDirection = Eigen::AngleAxisd(m_cfg.optimum_theta,Eigen::Vector3d::UnitY())*m_optimumDirection;
      m_optimumDirection = Eigen::AngleAxisd(m_cfg.optimum_phi,Eigen::Vector3d::UnitZ())*m_optimumDirection;

      m_cellid_converter = algorithms::GeoSvc::instance().cellIDPositionConverter();

    }

    void FarDetectorLinearTracking::process(
        const FarDetectorLinearTracking::Input& input,
        const FarDetectorLinearTracking::Output& output) const {

        const auto [inputhits,assocHits] = input;
        auto  [outputTracks,assocTracks] = output;

        // Check the number of input collections is correct
        int nCollections = inputhits.size();
        if(nCollections!=m_cfg.n_layer){
          error("Wrong number of input collections passed to algorithm");
          return;
        }

        std::vector<std::vector<Eigen::Vector3d>> convertedHits;
        std::vector<std::vector<edm4hep::MCParticle>> assocParts;

        // Check there aren't too many hits in any layer to handle
        // Temporary limit of number of hits per layer before Kalman filtering/GNN implemented
        // TODO - Implement more sensible solution
        for(const auto& layerHits: inputhits){
          if((*layerHits).size()>m_cfg.layer_hits_max){
            info("Too many hits in layer");
            return;
          }
          if((*layerHits).size()==0){
            trace("No hits in layer");
            return;
          }
          ConvertClusters(*layerHits,*assocHits,convertedHits,assocParts);
        }

        // Create a matrix to store the hit positions
        Eigen::MatrixXd hitMatrix(3,m_cfg.n_layer);

        // Create vector to store indexes of hits in the track
        std::vector<int> layerHitIndex(m_cfg.n_layer,0);

        int layer = 0;

        // Iterate over all combinations of measurements in the layers without recursion
        while(layer>=0){
          hitMatrix.col(layer) << convertedHits[layer][layerHitIndex[layer]];

          bool isValid = true;
          // Check the last two hits are within a certain angle of the optimum direction
          if(layer>0 && m_cfg.restrict_direction) {
            isValid = checkHitPair(hitMatrix.col(layer-1),hitMatrix.col(layer));
          }

          // If valid hit combination, move to the next layer or check the combination
          if(isValid){
            if(layer==m_cfg.n_layer-1){
              // Check the combination, if chi2 limit is passed, add the track to the output
              checkHitCombination(&hitMatrix,outputTracks,assocTracks,inputhits,assocParts,layerHitIndex);
            } else {
              layer++;
              continue;
            }
          }

          // Iterate current layer
          layerHitIndex[layer]++;

          bool doBreak = false;
          // Set up next combination to check
          while(layerHitIndex[layer]>=convertedHits[layer].size()){
            layerHitIndex[layer] = 0;
            if(layer==0){
              doBreak = true;
              break;
            }
            layer--;
            // Iterate previous layer
            layerHitIndex[layer]++;
          }
          if(doBreak) break;

        }

    }


    void FarDetectorLinearTracking::checkHitCombination(Eigen::MatrixXd* hitMatrix,
                                                       edm4eic::TrackCollection* outputTracks,
                                                       edm4eic::MCRecoTrackParticleAssociationCollection* assocTracks,
                                                       const std::vector<gsl::not_null<const edm4eic::Measurement2DCollection*>>& inputHits,
                                                       const std::vector<std::vector<edm4hep::MCParticle>>& assocParts,
                                                       const std::vector<int>& layerHitIndex) const {

      Eigen::Vector3d weightedAnchor = (*hitMatrix)*m_layerWeights/(m_layerWeights.sum());

      auto localMatrix = (*hitMatrix).colwise()-weightedAnchor;

      Eigen::BDCSVD<Eigen::MatrixXd> svd(localMatrix.transpose(), Eigen::ComputeThinU | Eigen::ComputeThinV);

      auto V = svd.matrixV();

      // Rotate into principle components and calculate chi2/ndf
      auto rotatedMatrix = localMatrix.transpose()*V;
      auto residuals     = rotatedMatrix.rightCols(2);
      double chi2        = (residuals.array()*residuals.array()).sum()/(2*m_cfg.n_layer);

      if(chi2>m_cfg.chi2_max) return;

      edm4hep::Vector3d outPos = weightedAnchor.data();
      edm4hep::Vector3d outVec = V.col(0).data();

      // Make sure fit was pointing in the right direction
      if(outVec.z>0) outVec = outVec*-1;

      int32_t type{0}; // Type of track
      edm4hep::Vector3f position(outPos.x,outPos.y,outPos.z);        // Position of the trajectory point [mm]
      edm4hep::Vector3f momentum(outVec.x,outVec.y,outVec.z);        // 3-momentum at the point [GeV]
      edm4eic::Cov6f    positionMomentumCovariance;                  // Error on the position
      float             time{0};                                     // Time at this point [ns]
      float             timeError{0};                                // Error on the time at this point
      float             charge{-1};                                  // Charge of the particle
      int32_t           ndf{m_cfg.n_layer-1};                        // Number of degrees of freedom
      int32_t           pdg{11};                                     // PDG code of the particle

      // Create the track
      auto track = (*outputTracks)->create(type,position,momentum,positionMomentumCovariance,time,timeError,charge,chi2,ndf,pdg);

      // Add Measurement2D relations and count occurance of particles contributing to the track
      std::unordered_map<const edm4hep::MCParticle*, int> particleCount;
      for (int layer = 0; layer < layerHitIndex.size(); layer++) {
        track.addToMeasurements((*inputHits[layer])[layerHitIndex[layer]]);
        const auto& assocParticle = assocParts[layer][layerHitIndex[layer]];
        particleCount[&assocParticle]++;
      }

      // Create track associations for each particle
      for (const auto& [particle, count] : particleCount) {
        auto trackAssoc = assocTracks->create();
        trackAssoc.setRec(track);
        trackAssoc.setSim(*particle);
        trackAssoc.setWeight(count/static_cast<double>(m_cfg.n_layer));
      }


    }

    // Check if a pair of hits lies close to the optimum direction
    bool FarDetectorLinearTracking::checkHitPair(const Eigen::Vector3d& hit1,
                                                const Eigen::Vector3d& hit2) const {

      Eigen::Vector3d hitDiff = hit2-hit1;
      hitDiff.normalize();

      double angle = std::acos(hitDiff.dot(m_optimumDirection));

      debug("Vector: x={}, y={}, z={}",hitDiff.x(),hitDiff.y(),hitDiff.z());
      debug("Optimum: x={}, y={}, z={}",m_optimumDirection.x(),m_optimumDirection.y(),m_optimumDirection.z());
      debug("Angle: {}, Tolerance {}",angle,m_cfg.step_angle_tolerance);

      if(angle>m_cfg.step_angle_tolerance) return false;

      return true;

    }

    // Convert measurements into global coordinates
    void FarDetectorLinearTracking::ConvertClusters( const edm4eic::Measurement2DCollection& clusters, const edm4eic::MCRecoTrackerHitAssociationCollection& assoc_hits, std::vector<std::vector<Eigen::Vector3d>>& pointPositions, std::vector<std::vector<edm4hep::MCParticle>>& assoc_parts ) const {

      // Get context of first hit
      const dd4hep::VolumeManagerContext* context = m_cellid_converter->findContext(clusters[0].getSurface());

      std::vector<Eigen::Vector3d> layerPositions;
      std::vector<edm4hep::MCParticle> assocParticles;

      for (auto cluster : clusters) {

        auto globalPos = context->localToWorld({cluster.getLoc()[0], cluster.getLoc()[1], 0});
        layerPositions.push_back(Eigen::Vector3d(globalPos.x()/ dd4hep::mm, globalPos.y()/ dd4hep::mm, globalPos.z()/ dd4hep::mm));

        // Determine the MCParticle associated with this measurement based on the weights
        // Get hit in measurement with max weight
        float maxWeight = 0;
        int maxIndex = -1;
        for (int i = 0; i < cluster.getWeights().size(); ++i) {
          if (cluster.getWeights()[i] > maxWeight) {
            maxWeight = cluster.getWeights()[i];
            maxIndex  = i;
          }
        }
        auto maxHit = cluster.getHits()[maxIndex];
        // Get associated raw hit
        auto rawHit = maxHit.getRawHit();

        // Loop over the hit associations to find the associated MCParticle
        for (const auto& hit_assoc : assoc_hits) {
          if (hit_assoc.getRawHit() == rawHit) {
            auto mcParticle = hit_assoc.getSimHit().getMCParticle();
            assocParticles.push_back(mcParticle);
            break;
          }
        }

      }

      pointPositions.push_back(layerPositions);
      assoc_parts.push_back(assocParticles);

    }


}
