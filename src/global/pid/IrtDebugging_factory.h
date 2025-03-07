// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

//#include <IRT/CherenkovDetectorCollection.h>
#include <JANA/JEvent.h>
//#include <edm4eic/CherenkovParticleIDCollection.h>
#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/pid/IrtDebugging.h"
//#include "algorithms/pid/IrtCherenkovParticleIDConfig.h"
#include "extensions/jana/JOmniFactory.h"
//#include "services/algorithms_init/AlgorithmsInit_service.h"
//#include "services/geometry/richgeo/RichGeo_service.h"

#include <edm4eic/RawTrackerHitCollection.h>

namespace eicrecon {

class IrtDebugging_factory :
    public JOmniFactory<IrtDebugging_factory/*, IrtCherenkovParticleIDConfig*/> {

private:
  using AlgoT = eicrecon::IrtDebugging;//CherenkovParticleID;
    std::unique_ptr<AlgoT> m_algo;

    PodioInput<edm4eic::TrackSegment> m_aerogel_tracks_input {this};
#if _TODAY_
    PodioInput<edm4eic::TrackSegment> m_gas_tracks_input {this};
    PodioInput<edm4eic::TrackSegment> m_merged_tracks_input {this};
#endif
  PodioInput<edm4hep::SimTrackerHit> m_sim_hits_input {this};
  //PodioInput<edm4eic::RawTrackerHit> m_raw_hits_input {this};
  //  PodioInput<edm4eic::MCRecoTrackerHitAssociation> m_raw_hit_assoc_input {this};
  //+PodioOutput<edm4eic::CherenkovParticleID> m_aerogel_particleIDs_output {this};
    PodioOutput<edm4eic::IrtDebugInfo> m_irt_debug_info_output {this};
#if _TODAY_
    PodioOutput<edm4eic::CherenkovParticleID> m_gas_particleIDs_output {this};

    ParameterRef<unsigned int> m_numRIndexBins {this, "numRIndexBins", config().numRIndexBins, ""};
    ParameterRef<std::vector<int>> m_pdgList {this, "pdgList", config().pdgList, ""};

    ParameterRef<double> m_aerogel_referenceRIndex {this, "aerogel:referenceRIndex", config().radiators["Aerogel"].referenceRIndex, ""};
    ParameterRef<double> m_aerogel_attenuation {this, "aerogel:attenuation", config().radiators["Aerogel"].attenuation, ""};
    ParameterRef<std::string> m_aerogel_smearingMode {this, "aerogel:smearingMode", config().radiators["Aerogel"].smearingMode, ""};
    ParameterRef<double> m_aerogel_smearing {this, "aerogel:smearing", config().radiators["Aerogel"].smearing, ""};

    ParameterRef<double> m_gas_referenceRIndex {this, "gas:referenceRIndex", config().radiators["Gas"].referenceRIndex, ""};
    ParameterRef<double> m_gas_attenuation {this, "gas:attenuation", config().radiators["Gas"].attenuation, ""};
    ParameterRef<std::string> m_gas_smearingMode {this, "gas:smearingMode", config().radiators["Gas"].smearingMode, ""};
    ParameterRef<double> m_gas_smearing {this, "gas:smearing", config().radiators["Gas"].smearing, ""};

    ParameterRef<bool> m_cheatPhotonVertex {this, "cheatPhotonVertex", config().cheatPhotonVertex, ""};
    ParameterRef<bool> m_cheatTrueRadiator {this, "cheatTrueRadiator", config().cheatTrueRadiator, ""};
#endif

  //+Service<AlgorithmsInit_service> m_algorithmsInit {this};
  //+Service<RichGeo_service> m_RichGeoSvc {this};

public:
    void Configure() {
        m_algo = std::make_unique<AlgoT>(GetPrefix());
        m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
#if _TODAY_
        m_algo->applyConfig(config());
        m_algo->init(m_RichGeoSvc().GetIrtGeo("DRICH")->GetIrtDetectorCollection(), logger());
#else
        m_algo->init(0);//m_RichGeoSvc().GetIrtGeo("DRICH")->GetIrtDetectorCollection(), logger());
#endif
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
#if 1//_TODAY_
      m_algo->process(
        {
          m_aerogel_tracks_input(),
	  m_sim_hits_input()//,
          //m_gas_tracks_input(),
          //m_merged_tracks_input(),
          //m_raw_hits_input(),
          //m_raw_hit_assoc_input()
        },
        {
          //m_aerogel_particleIDs_output().get()//,
          m_irt_debug_info_output().get()//,
          //m_gas_particleIDs_output().get()
	  }
      );
#endif
    }
};

} // eicrecon
