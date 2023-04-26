// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
// Converters of ParticleID objects
//

#pragma once

// data model
#include <edm4eic/CherenkovParticleID.h>
#include <edm4hep/ParticleID.h>

namespace eicrecon {

  class ConvertParticleID {
    public:

      // convert edm4eic::CherenkovParticleID hypotheses to list of edm4hep::ParticleID objects
      static std::vector<edm4hep::ParticleID> ConvertToParticleIDs(
          edm4eic::CherenkovParticleID in_pid,
          bool sort_by_likelihood = false
          ) {
        std::vector<edm4hep::ParticleID> out_pids;
        for(auto in_hyp : in_pid.getHypotheses()) {
          // scalars
          edm4hep::MutableParticleID out_pid{
            static_cast<decltype(edm4hep::ParticleIDData::type)>          (0), // FIXME: not used yet
            static_cast<decltype(edm4hep::ParticleIDData::PDG)>           (in_hyp.PDG),
            static_cast<decltype(edm4hep::ParticleIDData::algorithmType)> (0), // FIXME: not used yet
            static_cast<decltype(edm4hep::ParticleIDData::likelihood)>    (in_hyp.weight)
          };
          // parameters vector
          out_pid.addToParameters(in_hyp.npe); // NPE for this hypothesis
          // append
          out_pids.push_back(edm4hep::ParticleID(out_pid));
        }
        // sort output PIDs by likelihood
        if(sort_by_likelihood)
          std::sort(
              out_pids.begin(),
              out_pids.end(),
              [] (edm4hep::ParticleID& a, edm4hep::ParticleID&b) { return a.getLikelihood() > b.getLikelihood(); }
              );
        return out_pids;
      }

  }; // class ConvertParticleID
} // namespace eicrecon
