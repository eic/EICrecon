// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Christopher Dilks

// General algorithm to merge together particle ID datatypes

#pragma once

// data model
#include <algorithms/algorithm.h>
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <vector>

// EICrecon
#include "MergeParticleIDConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

  using MergeParticleIDAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      std::vector<const edm4eic::CherenkovParticleIDCollection>
    >,
    algorithms::Output<
      edm4eic::CherenkovParticleIDCollection
    >
  >;

  class MergeParticleID
  : public MergeParticleIDAlgorithm,
    public WithPodConfig<MergeParticleIDConfig> {

  public:
    MergeParticleID(std::string_view name)
      : MergeParticleIDAlgorithm{name,
                            {"inputTrackSegments"},
                            {"outputTrackSegments"},
                            "Effectively 'zip' the input particle IDs"} {}

    void init(std::shared_ptr<spdlog::logger>& logger);

    // - input: a list of particle ID collections, which we want to merge together
    // - output: the merged particle ID collection
    // - overload this function to support different collections from other PID subsystems, or to support
    //   merging PID results from overlapping subsystems
    void process(const Input&, const Output&) const final;

    private:
      std::shared_ptr<spdlog::logger> m_log;
  };
}
