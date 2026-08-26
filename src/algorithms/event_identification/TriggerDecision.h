
#pragma once
#include <vector>

// TODO: Add to edm4eic once ready
struct TriggerDecision {
    int detector_id;
    float timeslice_center;
    float timeslice_width;
};

struct TriggerDecisionCollection {
    std::vector<TriggerDecision*> contents;
};


