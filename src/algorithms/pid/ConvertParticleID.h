// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Christopher Dilks

// Converters of ParticleID objects
//

#pragma once

#include <cstddef>
#include <map>

// data model
#include <edm4eic/CherenkovParticleID.h>
#include <edm4eic/CherenkovParticleIDHypothesis.h>
#include <edm4hep/ParticleIDCollection.h>

namespace eicrecon {

class ConvertParticleID {
public:
  // index map, used for external access to the converted objects
  using IndexMap = std::map<std::size_t, unsigned int>; // <collection index, object ID>

  // convert edm4eic::CherenkovParticleID hypotheses to list of edm4hep::ParticleID objects
  // - requires input `CherenkovParticleID` object `in_pid`
  // - adds output `ParticleID` objects to collection `out_pids`
  // - sorted by likelihood, if `sort_by_likelihood==true`
  // - returns a map of the new `out_pid` indices to new `ParticleID` IDs, so the caller can
  //   access the newly created `ParticleID` objects
  static IndexMap ConvertToParticleIDs(const edm4eic::CherenkovParticleID& in_pid,
                                       edm4hep::ParticleIDCollection& out_pids,
                                       bool sort_by_likelihood = false) {

    // output vector of collection indices
    IndexMap out_indices;

    // build list of (hypothesis index, hypothesis weight)
    using HypIndex =
        std::pair<std::size_t, decltype(edm4eic::CherenkovParticleIDHypothesis::weight)>;
    std::vector<HypIndex> hyp_indices;
    for (std::size_t hyp_index = 0; hyp_index < in_pid.hypotheses_size(); hyp_index++)
      hyp_indices.push_back(HypIndex{hyp_index, in_pid.getHypotheses(hyp_index).weight});

    // sort it by likelihood, if needed
    if (sort_by_likelihood)
      std::sort(hyp_indices.begin(), hyp_indices.end(),
                [](HypIndex& a, HypIndex& b) { return a.second > b.second; });

    // create and fill output objects
    for (const auto& [hyp_index, hyp_weight] : hyp_indices) {

      // get the input hypothesis
      auto in_hyp = in_pid.getHypotheses(hyp_index);

      // create output `ParticleID` object
      auto out_index = out_pids.size();
      auto out_pid   = out_pids.create();
      out_indices.insert({out_index, out_pid.getObjectID().index});

      // fill scalars
      out_pid.setPDG(static_cast<decltype(edm4hep::ParticleIDData::PDG)>(in_hyp.PDG));
      out_pid.setLikelihood(
          static_cast<decltype(edm4hep::ParticleIDData::likelihood)>(in_hyp.weight));
      out_pid.setType(
          static_cast<decltype(edm4hep::ParticleIDData::type)>(0)); // FIXME: not used yet
      out_pid.setAlgorithmType(
          static_cast<decltype(edm4hep::ParticleIDData::algorithmType)>(0)); // FIXME: not used yet

      // fill parameters vector
      out_pid.addToParameters(static_cast<float>(in_hyp.npe)); // NPE for this hypothesis
    }

    return out_indices;
  }

}; // class ConvertParticleID
} // namespace eicrecon
