#ifndef EICRECON_JFACTORY_SILICONTRACKERDIGIT_H
#define EICRECON_JFACTORY_SILICONTRACKERDIGIT_H

#include <random>

#include <fmt/core.h>

#include <edm4hep/SimTrackerHit.h>
#include <edm4hep::RawTrackerHit>


class SimTrackerHitDigi {

public:
    SimTrackerHitDigi() = default;

    /** Event by event processing **/
    void execute() ;

    std::vector<const edm4hep::SimTrackerHit*> m_simhits; // inputs (set by caller before execute is called)
    std::vector<edm4hep::RawTrackerHit*>       m_hits;    // outputs (set by algorithm when execute is called)

    // This allows the caller to specify the type of objects being created for the output.
    // The default here would make plain old eicrecon::RawTrackerHit objects, but the user
    // could specify something that inherits from eicrecon::RawTrackerHit.
    // n.b. maybe could use variadic here for the arguments.
    std::function<eicrecon::RawTrackerHit()> new_RawTrackerHit = [](double one, double two, double three){return new eicrecon::RawTrackerHit(one,two,three); }

private:

    std::string m_config_prefix;    /// A prefix to use for command line parameters

};


#endif //EICRECON_JFACTORY_SILICONTRACKERDIGIT_H
