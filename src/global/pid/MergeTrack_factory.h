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
#include "extensions/jana/JChainMultifactoryT.h"
// services
#include "extensions/spdlog/SpdlogMixin.h"

namespace eicrecon {

  class MergeTrack_factory :
    public JChainMultifactoryT<NoConfig>,
    public SpdlogMixin
  {

    public:

      explicit MergeTrack_factory(
          std::string tag,
          const std::vector<std::string>& input_tags,
          const std::vector<std::string>& output_tags)
      : JChainMultifactoryT<NoConfig>(std::move(tag), input_tags, output_tags) {
        DeclarePodioOutput<edm4eic::TrackSegment>(GetOutputTags()[0]);
      }

      /** One time initialization **/
      void Init() override;

      /** Event by event processing **/
      void Process(const std::shared_ptr<const JEvent> &event) override;

    private:

      // underlying algorithm
      eicrecon::MergeTracks m_algo;
  };
}
