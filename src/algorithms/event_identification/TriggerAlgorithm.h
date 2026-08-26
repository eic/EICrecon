#pragma once
#include <algorithms/algorithm.h>
#include <edm4hep/EventHeaderCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include "TriggerDecision.h"

using TriggerAlgorithm = algorithms::Algorithm<
    algorithms::Input<
        edm4hep::EventHeaderCollection,
        std::vector<edm4eic::TrackerHitCollection>>,
        // TODO: Add additional inputs!
    algorithms::Output<TriggerDecisionCollection>>;
