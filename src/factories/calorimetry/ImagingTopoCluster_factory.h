// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include "algorithms/calorimetry/ImagingTopoCluster.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class ImagingTopoCluster_factory
    : public JOmniAlgorithmFactory<ImagingTopoCluster_factory, eicrecon::ImagingTopoCluster,
                                   ImagingTopoClusterConfig> {
private:
  //                                                    config().sameLayerDistXY};
  //                                                    config().diffLayerDistXY};
  ParameterRef<std::vector<double>> m_ldep_same{this, "sameLayerDistEtaPhi",
                                                config().sameLayerDistEtaPhi};
  ParameterRef<std::vector<double>> m_ldep_diff{this, "diffLayerDistEtaPhi",
                                                config().diffLayerDistEtaPhi};
  ParameterRef<std::vector<double>> m_ldtz_same{this, "sameLayerDistTZ", config().sameLayerDistTZ};
  ParameterRef<std::vector<double>> m_ldtz_diff{this, "diffLayerDistTZ", config().diffLayerDistTZ};
  //                                                config().sameLayerDistXYZ};
  //                                                config().diffLayerDistXYZ};
  ParameterRef<eicrecon::ImagingTopoClusterConfig::ELayerMode> m_sameLayerMode{
      this, "sameLayerMode", config().sameLayerMode};
  ParameterRef<eicrecon::ImagingTopoClusterConfig::ELayerMode> m_diffLayerMode{
      this, "diffLayerMode", config().diffLayerMode};

  ParameterRef<int> m_nlr{this, "neighbourLayersRange", config().neighbourLayersRange};
  ParameterRef<double> m_sd{this, "sectorDist", config().sectorDist};
  ParameterRef<double> m_mched{this, "minClusterHitEdep", config().minClusterHitEdep};
  ParameterRef<double> m_mcced{this, "minClusterCenterEdep", config().minClusterCenterEdep};
  ParameterRef<double> m_mced{this, "minClusterEdep", config().minClusterEdep};
  ParameterRef<std::size_t> m_mcnh{this, "minClusterNhits", config().minClusterNhits};
};

} // namespace eicrecon
