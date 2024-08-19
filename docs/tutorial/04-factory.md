
| title                                                                                  | teaching | exercises | questions                                            | objectives                                                                                                                                                                                                                                                                           | keypoints                                                                                 |
|----------------------------------------------------------------------------------------|----------|-----------|------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|-------------------------------------------------------------------------------------------|
| Creating or modifying a JANA factory in order to implement a reconstruction algorithm  | 15       | 20        | How to write a reconstruction algorithm in EICrecon? | Learn how to create a new factory in EICrecon that supplies a reconstruction algorithm for all to use.<br/><br/>Understand the directory structure for where the factory should be placed in the source tree.<br/><br/>Understand how to use a generic algorithm in a JANA factory.  | Create a factory for reconstructing single subdetector data or for global reconstruction. |

## Notice

**This section of the tutorial is outdated due to switch from `JFactory` to `JOmniFactory`. Refer to a [tutorial on reconstruction algorithms](https://eic.github.io/tutorial-reconstruction-algorithms/) for up-to-date information.**

## Introduction


Now that you've learned about JANA plugins and JEventProcessors, let's talk about JFactories. JFactories are another essential JANA component just like JEventProcessors and JEventSources. While JEventProcessors are used for _aggregating_ results from each event into a structured output such as a histogram or a file, JFactories are used for computing those results in an organized way.


### When do I use a JFactory?

- If you have an input file and need to read data model objects from it, use a JEventSource.
- If you have an output file (or histogram) and wish to write data model objects to it, use a JEventProcessor.
- If you have some data model objects and wish to produce a new data model object, use a JFactory.


### Why should I prefer writing a JFactory?

1. They make your code reusable. Different people can use your results later without having to understand the specifics of what you did.

2. If you are consuming some data which doesn't look right to you, JFactories make it extremely easy to pinpoint exactly which code produced this data.

3. EICrecon needs to run multithreaded, and using JFactories can help steer you away from introducing thorny parallelism bugs.

4. You can simply ask for the results you need and the JFactory will provide it. If nobody needs the results from the JFactory, it won't be run. If the results were already in the input file, it won't be run. If there are multiple consumers, the results are only computed once and then cached. If the JFactory relies on results from other JFactories, it will call them transparently and recursively.


### When do I create my own plugin?

- If you are doing a one-off prototype, it's fine to just use a ROOT macro.
- If you are writing code you'll probably return to, we recommend putting the code in a standalone (i.e. outside of the EICrecon source tree) plugin.
- If you are writing code other people will probably want to run, we recommend adding your plugin to the EICrecon source tree.
- If you are writing a JFactory, we recommend adding it to the EICrecon source tree, either to an existing plugin or to a new one.


## Algorithms vs Factories

In general, a Factory is a programming pattern for constructing objects in an abstract way. Oftentimes, the Factory is calling an algorithm under the hood.
This algorithm may be very generic. For instance, we may have a Factory that produces Cluster objects for a barrel calorimeter, and it calls a clustering algorithm
that doesn't care at all about barrel calorimeters, just the position and energy of energy of each CalorimeterHit object. Perhaps multiple factories for creating clusters
for completely different detectors are all using the same algorithm.

Note that Gaudi provides an abstraction called "Algorithm" which is essentially its own version of a JFactory. In EICrecon, we have been separating out _generic algorithms_ from the old Gaudi and new JANA code so that these can be developed and tested independently. To see an example of how a generic algorithm is being implemented, look at these examples:

src/detectors/EEMC/RawCalorimeterHit_factory_EcalEndcapNRawHits.h
src/algorithms/calorimetry/CalorimeterHitDigi.h
src/algorithms/calorimetry/CalorimeterHitDigi.cc

Using generic algorithms makes things slightly more complex. However, the generic algorithms can be recycled for use in multiple detector systems which adds some simplification.


## Parallelism considerations


JEventProcessors observe the entire _event stream_, and require a _critical section_ where only one thread is allowed to modify a shared resource (such as a histogram) at any time.
JFactories, on the other hand, only observe a single event at a time, and work on each event independently. Each worker thread is given an independent event with its own set of factories. This means that for a given JFactory instance, there will be only one thread working on one event at any time. You get the benefits of multithreading _without_ having to make each JFactory thread-safe.


You can write JFactories in an almost-functional style, but you can also cache some data on the JFactory that will stick around from event-to-event. This is useful for things like conditions and geometry data, where for performance reasons you don't want to be doing a deep lookup on every event. Instead, you can write callbacks such as `BeginRun()`, where you can update your cached values when the run number changes.


Note that just because the JFactory _can_ be called in parallel doesn't mean it always will. If you call event->Get() from inside `JEventProcessor::ProcessSequential`, in particular, the factory will run single-threaded and slow everything down. However, if you call it using `Prefetch` instead, it will run in parallel and you may get a speed boost.


## How do I use an existing JFactory? ##

Using an existing JFactory is extremely easy! Any time you are someplace where you have access to a `JEvent` object, do this:


~~~ c++

auto clusters = event->Get<edm4eic::Cluster>("EcalEndcapNIslandClusters");

for (auto c : clusters) {
  // ... do something with a cluster
}

~~~

As you can see, it doesn't matter whether the `Cluster` objects were calculated from some simpler objects, or were simply loaded from a file. This is a very powerful concept.

One thing we might want to do is to swap one factory for another, possibly even at runtime. This is easy to do if you just make the factory tag be a parameter:


~~~ c++

std::string my_cluster_source = "EcalEndcapNIslandClusters";  // Make this be a parameter
app->SetDefaultParameter("MyPlugin:MyAnalysis:my_cluster_source", my_cluster_source, "Cluster source for MyAnalysis");
auto clusters = event->Get<edm4eic::Cluster>(my_cluster_source);
~~~

## How do I create a new JFactory?

We are going to add a new JFactory inside EICrecon.

`src/detectors/EEMC/Cluster_factory_EcalEndcapNIslandClusters.h`:
~~~ c++
#pragma once

#include <edm4eic/Cluster.h>
#include <JANA/JFactoryT.h>
#include <services/log/Log_service.h>

class Cluster_factory_EcalEndcapNIslandClusters : public JFactoryT<edm4eic::Cluster> {
public:

    Cluster_factory_EcalEndcapNIslandClusters(); // Constructor

    void Init() override;
    // Gets called exactly once at the beginning of the JFactory's life

    void ChangeRun(const std::shared_ptr<const JEvent> &event) override {};
    // Gets called on events where the run number has changed (before Process())

    void Process(const std::shared_ptr<const JEvent> &event) override;
    // Gets called on every event

    void Finish() override {};
    // Gets called exactly once at the end of the JFactory's life

private:
    float m_scaleFactor;
    std::shared_ptr<spdlog::logger> m_log;

};

~~~

`src/detectors/EEMC/Cluster_factory_EcalEndcapNIslandClusters.cc`:
~~~ c++
#include "Cluster_factory_EcalEndcapNIslandClusters.h"

#include <edm4eic/ProtoCluster.h>
#include <JANA/JEvent.h>


Cluster_factory_EcalEndcapNIslandClusters::Cluster_factory_EcalEndcapNIslandClusters() {

    SetTag("EcalEndcapNIslandClusters");
}


void Cluster_factory_EcalEndcapNIslandClusters::Init() {
    auto app = GetApplication();

    // This is an example of how to declare a configuration parameter that
    // can be set at run time. e.g. with -PEEMC:EcalEndcapNIslandClusters:scaleFactor=0.97
    m_scaleFactor =0.98;
    app->SetDefaultParameter("EEMC:EcalEndcapNIslandClusters:scaleFactor", m_scaleFactor, "Energy scale factor");

    // This is how you access shared resources using the JService interface
    m_log = app->GetService<Log_service>()->logger("EcalEndcapNIslandClusters");
}


void Cluster_factory_EcalEndcapNIslandClusters::Process(const std::shared_ptr<const JEvent> &event) {

    m_log->info("Processing event {}", event->GetEventNumber());

    // Grab inputs
    auto protoclusters = event->Get<edm4eic::ProtoCluster>("EcalEndcapNIslandProtoClusters");

    // Loop over protoclusters and turn each into a cluster
    std::vector<edm4eic::Cluster*> outputClusters;
    for( auto proto : protoclusters ) {

        // ======================
        // Algorithm goes here!
        // ======================

        auto cluster = new edm4eic::Cluster(
            0, // type
            energy * m_scaleFactor,
            sqrt(energyError_squared),
            time,
            timeError,
            proto->hits_size(),
            position,
            edm4eic::Cov3f(), // positionError,
            0.0,              // intrinsicTheta,
            0.0,              // intrinsicPhi,
            edm4eic::Cov2f()  // intrinsicDirectionError
            );

        outputClusters.push_back( cluster );
    }

    // Hand ownership of algorithm objects over to JANA
    Set(outputClusters);
}


~~~

We can now fill in the algorithm with anything we like!


~~~ c++
        // Grab inputs
        auto protoclusters = event->Get<edm4eic::ProtoCluster>("EcalEndcapNIslandProtoClusters");

        // Loop over protoclusters and turn each into a cluster
        std::vector<edm4eic::Cluster*> outputClusters;
        for( auto proto : protoclusters ) {

            // Fill cumulative values by looping over all hits in proto cluster
            float energy = 0;
            double energyError_squared = 0.0;
            float time = 1.0E8;
            float timeError;
            edm4hep::Vector3f position;
            double sum_weights = 0.0;
            for( uint32_t ihit=0; ihit<proto->hits_size() ; ihit++){
                auto const &hit = proto->getHits(ihit);
                auto weight = proto->getWeights(ihit);
                energy += hit.getEnergy();
                energyError_squared += std::pow(hit.getEnergyError(), 2.0);
                if( hit.getTime() < time ){
                    time = hit.getTime();            // use earliest time
                    timeError = hit.getTimeError();  // use error of earliest time
                }
                auto &p = hit.getPosition();
                position.x += p.x*weight;
                position.y += p.y*weight;
                position.z += p.z*weight;
                sum_weights += weight;
            }

            // Normalize position
            position.x /= sum_weights;
            position.y /= sum_weights;
            position.z /= sum_weights;

            // Create a cluster object from values accumulated from hits above
            auto cluster = new edm4eic::Cluster(
                0, // type (?))
                energy * m_scaleFactor,
                sqrt(energyError_squared),
                time,
                timeError,
                proto->hits_size(),
                position,

                // Not sure how to calculate these last few
                edm4eic::Cov3f(), // positionError,
                0.0, // intrinsicTheta,
                0.0, // intrinsicPhi,
                edm4eic::Cov2f() // intrinsicDirectionError
                );

            outputClusters.push_back( cluster );
        }

        // Hand ownership of algorithm objects over to JANA
        Set(outputClusters);

~~~


You can't pass JANA a JFactory directly (because it needs to create an arbitrary number of them on the fly). Instead you register a `JFactoryGenerator` object:

`src/detectors/EEMC/EEMC.cc`
~~~ c++

// In your plugin's init

#include <JANA/JFactoryGenerator.h>
// ...
#include "Cluster_factory_EcalEndcapNIslandClusters.h"

extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        // ...

        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalEndcapNIslandClusters>());
     }

~~~

Finally, we go ahead and trigger the factory (remember, factories won't do anything unless activated by a JEventProcessor). You can open the

~~~ bash
eicrecon in.root -Ppodio:output_file=out.root -Ppodio:output_collections=EcalEndcapNIslandClusters -Pjana:nevents=10
~~~


Your exercise is to get this JFactory working! You can tweak the algorithm, add log messages, add additional config parameters, etc.



{% include links.md %}
