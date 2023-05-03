// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
// Converters of ParticleID objects
//

#pragma once

#include <cstddef>

// data model
#include <edm4eic/CherenkovParticleID.h>
#include <edm4eic/CherenkovParticleIDHypothesis.h>
#include <edm4hep/ParticleIDCollection.h>

namespace eicrecon {

  class ConvertParticleID {
    public:

      // convert edm4eic::CherenkovParticleID hypotheses to list of edm4hep::ParticleID objects
      static std::unique_ptr<edm4hep::ParticleIDCollection> ConvertToParticleIDs(
          edm4eic::CherenkovParticleID in_pid,
          bool sort_by_likelihood = false
          ) {

        // start output collection, to persistify objects
        auto out_pids = std::make_unique<edm4hep::ParticleIDCollection>();

        // build list of (hypothesis index, hypothesis weight)
        using HypIndex = std::pair<std::size_t, decltype(edm4eic::CherenkovParticleIDHypothesis::weight)>;
        std::vector<HypIndex> hyp_indices;
        for(std::size_t hyp_index=0; hyp_index<in_pid.hypotheses_size(); hyp_index++)
          hyp_indices.push_back(HypIndex{
              hyp_index,
              in_pid.getHypotheses(hyp_index).weight
              });

        // sort it by likelihood, if needed
        if(sort_by_likelihood)
          std::sort(
              hyp_indices.begin(),
              hyp_indices.end(),
              [] (HypIndex& a, HypIndex& b) { return a.second > b.second; }
              );

        // create and fill output objects
        for(const auto& [hyp_index, hyp_weight] : hyp_indices) {

          auto in_hyp  = in_pid.getHypotheses(hyp_index);
          auto out_pid = out_pids->create();

          // fill scalars
          out_pid.setPDG(           static_cast<decltype(edm4hep::ParticleIDData::PDG)>           (in_hyp.PDG)    );
          out_pid.setLikelihood(    static_cast<decltype(edm4hep::ParticleIDData::likelihood)>    (in_hyp.weight) );
          out_pid.setType(          static_cast<decltype(edm4hep::ParticleIDData::type)>          (0)             ); // FIXME: not used yet
          out_pid.setAlgorithmType( static_cast<decltype(edm4hep::ParticleIDData::algorithmType)> (0)             ); // FIXME: not used yet

          // fill parameters vector
          out_pid.addToParameters( static_cast<float> (in_hyp.npe) ); // NPE for this hypothesis

        }

        return out_pids;
      }

  }; // class ConvertParticleID
} // namespace eicrecon
