// Copyright (C) 2022, 2023 Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// algorithms
#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/pid/MergeTracks.h"
// JANA
#include "extensions/jana/JOmniFactory.h"
// services
#include "extensions/spdlog/SpdlogMixin.h"

namespace eicrecon {


class MergeTrack_factory : public JOmniFactory<MergeTrack_factory> {
private:

    // Underlying algorithm
    std::unique_ptr<eicrecon::MergeTracks> m_algo;

    // Declare inputs
    VariadicPodioInput<edm4eic::TrackSegment> m_track_segments_input {this};

    // Declare outputs
    PodioOutput<edm4eic::TrackSegment> m_track_segments_output {this};

public:
    void Configure() {
        m_algo->init(logger());
    }

    void ChangeRun(int64_t run_number) { }

    void Process(int64_t run_number, uint64_t event_number) {
        auto in1 = m_track_segments_input();
        std::vector<gsl::not_null<const edm4eic::TrackSegmentCollection*>> in2;
        std::copy(in1.cbegin(), in1.cend(), std::back_inserter(in2));

        m_algo->process({in2}, {m_track_segments_output().get()});
    }

  };
}
