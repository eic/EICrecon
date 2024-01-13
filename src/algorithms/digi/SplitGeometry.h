// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#pragma once

#include <edm4eic/RawTrackerHitCollection.h>
#include <spdlog/spdlog.h>
//#include <DDSegmentation/BitFieldCoder.h>
#include <DD4hep/Detector.h>

#include "SplitGeometryConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

    class SplitGeometry : public WithPodConfig<SplitGeometryConfig>  {

    public:
        void init(std::shared_ptr<spdlog::logger>& logger);
	std::vector<std::unique_ptr<edm4eic::RawTrackerHitCollection>> process(const edm4eic::RawTrackerHitCollection& sim_hits);
	//Ideally should be able to take anything with a dd4hep cellid

    private:
        const dd4hep::Detector*         m_detector{nullptr};
        const dd4hep::BitFieldCoder*    m_id_dec{nullptr};
        std::shared_ptr<spdlog::logger> m_log;

	int m_division_idx;

    };

} // eicrecon
