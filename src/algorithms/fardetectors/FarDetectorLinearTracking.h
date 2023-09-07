// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner

#pragma once

#include <DDRec/CellIDPositionConverter.h>
#include <Eigen/Dense>
// Event Model related classes
#include <edm4eic/TrackParametersCollection.h>

#include <spdlog/logger.h>
#include "FarDetectorLinearTrackingConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

typedef std::map<int,std::vector<edm4hep::TrackerHit>> LayerMap;

namespace eicrecon {

    class FarDetectorLinearTracking : public WithPodConfig<FarDetectorLinearTrackingConfig>  {

    public:

        /** One time initialization **/
        void init(const dd4hep::Detector* det,
		  std::shared_ptr<spdlog::logger>& logger);

        /** Event by event processing **/
        std::unique_ptr<edm4eic::TrackParametersCollection> produce(const edm4hep::TrackerHitCollection &inputhits);


    private:
        const dd4hep::Detector*         m_detector{nullptr};
        const dd4hep::BitFieldCoder*    m_id_dec{nullptr};
        std::shared_ptr<spdlog::logger> m_log;

        int m_module_idx{0};
        int m_layer_idx{0};
	Eigen::VectorXd m_layerWeights;


	bool checkLayerHitLimits(LayerMap hits);

	void makeHitCombination(int level,
				Eigen::MatrixXd* hitMatrix, 
				std::vector<int> layerKeys,
				LayerMap hits, 
				std::unique_ptr<edm4eic::TrackParametersCollection>* outputTracks);

	void checkHitCombination(Eigen::MatrixXd* hitMatrix, 
				 std::unique_ptr<edm4eic::TrackParametersCollection>* outputTracks);
    };

} // eicrecon
