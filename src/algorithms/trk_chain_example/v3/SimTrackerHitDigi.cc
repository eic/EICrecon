
#include <SimTrackerHitDigi.h>

void SimTrackerHitDigi::execute() {
    /** Event by event processing **/

    for( auto simhit : simhits ) {
        hits.push_back( new_RawTrackerHit(1, 2, 3) );  // (values would be smeared forms of simhit)
    }
}


