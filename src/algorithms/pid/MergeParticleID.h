// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Christopher Dilks

// General algorithm to merge together particle ID datatypes

#pragma once

// data model
#include <algorithms/algorithm.h>
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

// EICrecon
#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/pid/MergeParticleIDConfig.h"
#include "algorithms/pid/Tools.h"

namespace eicrecon {

using MergeParticleIDAlgorithm = algorithms::Algorithm<
    algorithms::Input<std::vector<const edm4eic::CherenkovParticleIDCollection>>,
    algorithms::Output<edm4eic::CherenkovParticleIDCollection>>;

class MergeParticleID : public MergeParticleIDAlgorithm,
                        public WithPodConfig<MergeParticleIDConfig> {

public:
  MergeParticleID(std::string_view name)
      : MergeParticleIDAlgorithm{name,
                                 {"inputTrackSegments"},
                                 {"outputTrackSegments"},
                                 "Effectively 'zip' the input particle IDs"} {}

  void init();

  // - input: a list of particle ID collections, which we want to merge together
  // - output: the merged particle ID collection
  // - overload this function to support different collections from other PID subsystems, or to support
  //   merging PID results from overlapping subsystems
  void process(const Input&, const Output&) const final;

private:
  template <algorithms::LogLevel lvl, typename... T>
  constexpr void log(fmt::format_string<T...> fmt, T&&... args) const {
    log<lvl>(fmt, std::forward<decltype(args)>(args)...);
  }

  friend class MergeParticleIDConfig;
  friend class Tools;
};

// Definition of MergeParticleIDConfig::Print requires class MergeParticleID, but
// circular dependency prevents it from being in MergeParticleIDConfig.h
template <algorithms::LogLevel lvl>
constexpr void MergeParticleIDConfig::Print(const MergeParticleID* logger) const {
  // print all parameters
  logger->log<lvl>("{:=^60}", " MergeParticleIDConfig Settings ");
  auto print_param = [&logger](auto name, auto val) {
    logger->log<lvl>("  {:>20} = {:<}", name, val);
  };
  print_param("mergeMode", mergeMode);
  logger->log<lvl>("{:=^60}", "");
}

} // namespace eicrecon
