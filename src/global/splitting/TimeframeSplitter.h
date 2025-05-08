// Copyright 2024, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <JANA/Utils/JEventLevel.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/EventHeaderCollection.h>
#include "services/io/podio/datamodel_glue.h"
#include <JANA/JEventUnfolder.h>

struct TimeframeSplitter : public JEventUnfolder {

  PodioInput<edm4hep::EventHeader> m_event_header_in{this,
                                                     {.name = "EventHeader", .is_optional = true}};
  PodioOutput<edm4hep::EventHeader> m_event_header_out{this, "EventHeader"};

  VariadicPodioInput<edm4hep::SimTrackerHit> m_simtrackerhits_in{
      this, {.names = {"MPGDBarrelHits", "SiBarrelHits", "TOFEndcapHits"}, .is_optional = true}};

  VariadicPodioOutput<edm4hep::SimTrackerHit> m_simtrackerhits_out{
      this, {"MPGDBarrelHits", "SiBarrelHits", "TOFEndcapHits"}};

  TimeframeSplitter() {
    SetTypeName(NAME_OF_THIS);
    SetParentLevel(JEventLevel::Timeslice);
    SetChildLevel(JEventLevel::PhysicsEvent);
  }

  Result Unfold(const JEvent& parent, JEvent& child, int child_idx) override {

    LOG_INFO(GetLogger()) << "Running TimeframeSplitter::Unfold() on timeslice #"
                          << parent.GetEventNumber() << LOG_END;

    // For now, a one-to-one relationship between timeslices and events
    child.SetEventNumber(parent.GetEventNumber());
    child.SetRunNumber(parent.GetRunNumber());

    // Insert an EventHeader object into the child event
    // For now this is just a ref to the timeslice header
    if (m_event_header_in() != nullptr) {
      m_event_header_out()->setSubsetCollection(true);
      m_event_header_out()->push_back(m_event_header_in()->at(0));
    }

    // Insert all SimTrackerHits into the child event

    for (size_t coll_index = 0; coll_index < m_simtrackerhits_in().size(); ++coll_index) {

      const auto* coll_in = m_simtrackerhits_in().at(coll_index);
      auto& coll_out      = m_simtrackerhits_out().at(coll_index);
      if (coll_in != nullptr) {
        coll_out->setSubsetCollection(true);
        for (const auto& hit : *coll_in) {
          coll_out->push_back(hit);
        }
      }
    }

    // Produce exactly one physics event per timeframe for now
    return JEventUnfolder::Result::NextChildNextParent;
  }
};
