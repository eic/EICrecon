// Copyright (C) 2022, 2023 Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

// JANA
#include "extensions/jana/JChainFactoryT.h"
#include <JANA/JEvent.h>

// data model
#include <edm4eic/TrackSegmentCollection.h>

// algorithms
#include "algorithms/pid/MergeTracks.h"

// services
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "extensions/spdlog/SpdlogMixin.h"

namespace eicrecon {

  class MergeTracks;

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
