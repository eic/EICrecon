// Copyright (C) 2022, 2023 Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <JANA/JEvent.h>
#include <JANA/JException.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <cstddef>
#include <memory>
#include <string>
#include <typeindex>
#include <utility>
#include <vector>

// algorithms
#include "algorithms/pid/MergeTracks.h"
// JANA
#include "extensions/jana/JChainFactoryT.h"
// services
#include "extensions/spdlog/SpdlogMixin.h"

namespace eicrecon {

  class MergeTrack_factory :
    public JChainFactoryT<edm4eic::TrackSegment>,
    public SpdlogMixin
  {

    public:

      explicit MergeTrack_factory(std::vector<std::string> default_input_tags) :
        JChainFactoryT<edm4eic::TrackSegment>(std::move(default_input_tags)) {}

      /** One time initialization **/
      void Init() override;

      /** On run change preparations **/
      void BeginRun(const std::shared_ptr<const JEvent> &event) override;

      /** Event by event processing **/
      void Process(const std::shared_ptr<const JEvent> &event) override;

    private:

      // underlying algorithm
      eicrecon::MergeTracks m_algo;
  };
}
