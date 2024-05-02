// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner

#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3f.h>
#include <edm4eic/Cov2f.h>
#include <edm4eic/Cov3f.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4hep/utils/vector_utils.h>
#include <edm4eic/vector_utils.h>
#include <edm4hep/TrackerHitCollection.h>

#include "FarDetectorLinearTracking.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include <iterator>
#include <algorithm>
#include <map>



namespace eicrecon {

    void FarDetectorLinearTracking::init(std::shared_ptr<spdlog::logger>& logger) {

      m_log      = logger;

      // For changing how strongly each layer hit is in contributing to the fit
      m_layerWeights = Eigen::VectorXd::Constant(m_cfg.n_layer,1);

    }

    void FarDetectorLinearTracking::process(
        const FarDetectorLinearTracking::Input& input,
        const FarDetectorLinearTracking::Output& output) const {

        const auto [inputhits] = input;
        auto [outputTracks] = output;

        // Check the number of input collections is correct
        int nCollections = inputhits.size();
        if(nCollections!=m_cfg.n_layer){
          m_log->error("Wrong number of input collections passed to algorithm");
          return;
        }

        // Check there aren't too many hits in any layer to handle
        // Temporary limit of number of hits per layer before Kalman filtering/GNN implemented
        // TODO - Implement more sensible solution
        for(const auto& layerHits: inputhits){
          if((*layerHits).size()>m_cfg.layer_hits_max){
            m_log->info("Too many hits in layer");
            return;
          }
        }

        // Create a matrix to store the hit positions
        Eigen::MatrixXd hitMatrix(3,m_cfg.n_layer);
        // Loop over all combinations of hits fitting a track to all layers
        makeHitCombination(m_cfg.n_layer-1,&hitMatrix,inputhits,outputTracks);

    }


    void FarDetectorLinearTracking::makeHitCombination(int level,
                                                       Eigen::MatrixXd* hitMatrix,
                                                       const std::vector<gsl::not_null<const edm4hep::TrackerHitCollection*>>& hits,
                                                       gsl::not_null<edm4eic::TrackSegmentCollection*> outputTracks ) const {

      // Iterate over hits in this layer
      for(auto hit : (*hits[level])){
        auto pos = hit.getPosition();
        hitMatrix->col(level) << pos.x, pos.y, pos.z;

        if(level>0){
          makeHitCombination(level-1,
                             hitMatrix,
                             hits,
                             outputTracks);
        }
        else{
          checkHitCombination(hitMatrix,outputTracks);
        }
      }

    }


    void FarDetectorLinearTracking::checkHitCombination(Eigen::MatrixXd* hitMatrix,
                                                        gsl::not_null<edm4eic::TrackSegmentCollection*> outputTracks ) const {

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

      uint64_t          surface{0};     // Surface track was propagated to (possibly multiple per detector)
      uint32_t          system{0};      // Detector system track was propagated to
      edm4hep::Vector3f position(outPos.x/dd4hep::mm,outPos.y/dd4hep::mm,outPos.z/dd4hep::mm);        // Position of the trajectory point [mm]
      edm4eic::Cov3f    positionError;  // Error on the position
      edm4hep::Vector3f momentum = {(float)outVec.x,(float)outVec.y,(float)outVec.z};       // 3-momentum at the point [GeV]
      edm4eic::Cov3f    momentumError;  // Error on the 3-momentum
      float             time{0};        // Time at this point [ns]
      float             timeError{0};   // Error on the time at this point
      float             theta = edm4eic::anglePolar(outVec);          // polar direction of the track at the surface [rad]
      float             phi   = edm4eic::angleAzimuthal(outVec);         // azimuthal direction of the track at the surface [rad]
      edm4eic::Cov2f    directionError;  // Error on the polar and azimuthal angles
      float             pathlength{0};      // Pathlength from the origin to this point
      float             pathlengthError{0}; // Error on the pathlength

      edm4eic::TrackPoint point({surface,system,position,positionError,momentum,momentumError,time,timeError,theta,phi,directionError,pathlength,pathlengthError});
      
      float length      = 0;
      float lengthError = 0;
      auto  segment     = (*outputTracks)->create(length,lengthError);

      segment.addToPoints(point);


    }

}
