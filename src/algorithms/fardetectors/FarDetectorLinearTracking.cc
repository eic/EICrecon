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


    void FarDetectorLinearTracking::init(const dd4hep::Detector* det, std::shared_ptr<spdlog::logger>& logger) {

      m_detector = det;
      m_log      = logger;

      if (m_cfg.readout.empty()) {
        throw JException("Readout is empty");
      }

      try {
        m_id_dec = det->readout(m_cfg.readout).idSpec().decoder();
        if (!m_cfg.moduleField.empty()) {
          m_module_idx = m_id_dec->index(m_cfg.moduleField);
          m_log->debug("Find module field {}, index = {}", m_cfg.moduleField, m_module_idx);
        }
        if (!m_cfg.layerField.empty()) {
          m_layer_idx = m_id_dec->index(m_cfg.layerField);
          m_log->debug("Find layer field {}, index = {}", m_cfg.layerField, m_layer_idx);
        }
      } catch (...) {
        m_log->error("Failed to load ID decoder for {}", m_cfg.readout);
        throw JException("Failed to load ID decoder");
      }

      // For changing how strongly each layer hit is in contributing to the fit
      m_layerWeights = Eigen::VectorXd::Constant(m_cfg.n_layer,1);

    }

    std::unique_ptr<edm4eic::TrackSegmentCollection> FarDetectorLinearTracking::process(const edm4hep::TrackerHitCollection &inputhits) {

        auto outputTracks = std::make_unique<edm4eic::TrackSegmentCollection>();

        std::map<int,LayerMap> sortedHits;

        // Sort the hits by module and layer
        for(auto hit: inputhits){
          auto module = m_id_dec->get(hit.getCellID(),m_module_idx);
          auto layer  = m_id_dec->get(hit.getCellID(),m_layer_idx);
          sortedHits[module][layer].push_back(hit);
        }

        // Loop over module
        for ( auto &[key,moduleHits] : sortedHits ) {

          // Check the number of hits in the module/layer is appropriate for the algortihm
          if(!checkLayerHitLimits(moduleHits)) continue;

          std::vector<int> layerKeys;
          for( auto &[key2,layerHits] : moduleHits){
            layerKeys.push_back(key2);
          }

          //double meanWeight = 1;

          Eigen::MatrixXd hitMatrix(3,m_cfg.n_layer);
          makeHitCombination(m_cfg.n_layer-1,&hitMatrix,layerKeys,moduleHits,&outputTracks);

        }

        return outputTracks;

    }

    // Check there is
    bool FarDetectorLinearTracking::checkLayerHitLimits(LayerMap hits){
      // Check there is a hit in each layer of the module
      if(hits.size()<m_cfg.n_layer) return 0;

      // Check there aren't too many hits in any layer to handle
      // Temporary limit of number of hits per layer before Kalman filtering/GNN implemented
      // TODO - Implement more sensible solution
      for(auto &[key,layerHits]: hits){
        if(layerHits.size()>m_cfg.layer_hits_max) return 0;
      }
      return 1;
    }

    void FarDetectorLinearTracking::makeHitCombination(int level,
                                                       Eigen::MatrixXd* hitMatrix,
                                                       std::vector<int> layerKeys,
                                                       LayerMap hits,
                                                       std::unique_ptr<edm4eic::TrackSegmentCollection>* outputTracks ){

      // Iterate over hits in this layer
      for(auto hit : hits[layerKeys[level]]){
        auto pos = hit.getPosition();
        hitMatrix->col(level) << pos.x, pos.y, pos.z;

        if(level>0){
          makeHitCombination(level-1,
                             hitMatrix,
                             layerKeys,
                             hits,
                             outputTracks);
        }
        else{
          checkHitCombination(hitMatrix,outputTracks);
        }
      }

    }


    void FarDetectorLinearTracking::checkHitCombination(Eigen::MatrixXd* hitMatrix,
                                                        std::unique_ptr<edm4eic::TrackSegmentCollection>* outputTracks ){

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
      auto segment = (*outputTracks)->create(0,0);

      segment.addToPoints(point);


    }

}
