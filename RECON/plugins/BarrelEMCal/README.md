

Here are a couple of examples for how we may implement the EDM4hep classes
and names.

option 1: Use the factory tag as the collection name.

option 2: Create a new class named using the collection name and inheriting from
the actual edm4hep class.

For the example, we implement the Juggler algorithm found here:

https://eicweb.phy.anl.gov/EIC/juggler/-/blob/master/JugDigi/src/components/CalorimeterHitDigi.cpp

This takes in objects of type _edm4hep::SimCalorimeterHit_ and produces
objects of type _edm4hep::RawCalorimeterHit_. The Gaudi algorithm
is generic so that it can be used with any calorimeter detector.
For puposes of illustration, I focus on the Barrel EMCAL detector.

Note that the simulated data file can be probed for the collection names and
corresponding data types like this:

~~~
jana -PPLUGINS=jana_edm4hep -pPODIO:PRINT_TYPE_TABLE=1 file.root
~~~

The output of which includes a table like the one below. From there,
you can see several collections with data type _edm4hep::SimCalorimeterHit_.
One of these is the collection named _EcalBarrelHits_. The goal of
our algorithm will therefore be to take the _EcalBarrelHits_ collection
as input and create objects of type _edm4hep::RawCalorimeterHit_ as
output, but identified as coming from the Barrel EMCal. How to do this?
There are a couple of options:

Option 1:<br>
Define a JFactory that produces objects of type _edm4hep::RawCalorimeterHit_
but has a factory tag with a name like "EcalBarrelRawCalorimeterHits"

Option 2:<br>
Define a C++ class called "EcalBarrelRawCalorimeterHit" that inherits from
_edm4hep::RawCalorimeterHit_ and make the factory produce that.

The benefit of Option 1 is that it matches closer to how the objects from
the event source are accessed (i.e. using the podio collection name as the
factory tag). It does so at the expense of preventing the factory tag from
being used for its intended purpose of allowing alternative algorithms.
The benefit of Option 2 is that it allows the factory tags to be used as
intended.

Users would access the data from these two options like this:
~~~
# option 1:
auto event->Get<edm4hep::RawCalorimeterHit>("EcalBarrelRawCalorimeterHits");

# option 2:
auto event->Get<EcalBarrelRawCalorimeterHit>(); // n.b. EcalBarrelRawCalorimeterHit isA edm4hep::RawCalorimeterHit
~~~

The original algorithm has been ported to the _JFactory_EcalBarrelRawCalorimeterHit_
class. The _JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits_ class would
be mostly identical, so only those few places where they differ are filled out for
illustrative purposes.

~~~
Available Collections

Collection Name                      Data Type
-----------------------------------  -----------------------------------
B0PreshowerHits                      edm4hep::SimTrackerHit
B0TrackerHits                        edm4hep::SimTrackerHit
DIRCBarHits                          edm4hep::SimTrackerHit
DRICHHits                            edm4hep::SimTrackerHit
EcalBarrelHits                       edm4hep::SimCalorimeterHit
EcalBarrelHitsContributions          edm4hep::CaloHitContribution
EcalEndcapNHits                      edm4hep::SimCalorimeterHit
EcalEndcapNHitsContributions         edm4hep::CaloHitContribution
EcalEndcapPHits                      edm4hep::SimCalorimeterHit
EcalEndcapPHitsContributions         edm4hep::CaloHitContribution
EventHeader                          edm4hep::EventHeader
ForwardOffMTrackerHits1              edm4hep::SimTrackerHit
ForwardOffMTrackerHits2              edm4hep::SimTrackerHit
ForwardOffMTrackerHits3              edm4hep::SimTrackerHit
ForwardOffMTrackerHits4              edm4hep::SimTrackerHit
ForwardRomanPotHits1                 edm4hep::SimTrackerHit
ForwardRomanPotHits2                 edm4hep::SimTrackerHit
GEMTrackerEndcapHits1                edm4hep::SimTrackerHit
GEMTrackerEndcapHits2                edm4hep::SimTrackerHit
GEMTrackerEndcapHits3                edm4hep::SimTrackerHit
HcalBarrelHits                       edm4hep::SimCalorimeterHit
HcalBarrelHitsContributions          edm4hep::CaloHitContribution
HcalEndcapNHits                      edm4hep::SimCalorimeterHit
HcalEndcapNHitsContributions         edm4hep::CaloHitContribution
HcalEndcapPHits                      edm4hep::SimCalorimeterHit
HcalEndcapPHitsContributions         edm4hep::CaloHitContribution
MCParticles                          edm4hep::MCParticle
MPGDTrackerBarrelHits1               edm4hep::SimTrackerHit
MPGDTrackerBarrelHits2               edm4hep::SimTrackerHit
MRICHHits                            edm4hep::SimTrackerHit
TOFBarrelHits                        edm4hep::SimTrackerHit
TaggerCalorimeter1Hits               edm4hep::SimCalorimeterHit
TaggerCalorimeter1HitsContributions  edm4hep::CaloHitContribution
TaggerCalorimeter2Hits               edm4hep::SimCalorimeterHit
TaggerCalorimeter2HitsContributions  edm4hep::CaloHitContribution
TaggerTracker1Hits                   edm4hep::SimTrackerHit
TaggerTracker2Hits                   edm4hep::SimTrackerHit
TrackerBarrelHits                    edm4hep::SimTrackerHit
TrackerEndcapHits1                   edm4hep::SimTrackerHit
TrackerEndcapHits2                   edm4hep::SimTrackerHit
TrackerEndcapHits3                   edm4hep::SimTrackerHit
TrackerEndcapHits4                   edm4hep::SimTrackerHit
TrackerEndcapHits5                   edm4hep::SimTrackerHit
TrackerEndcapHits6                   edm4hep::SimTrackerHit
VertexBarrelHits                     edm4hep::SimTrackerHit
ZDCEcalHits                          edm4hep::SimCalorimeterHit
ZDCEcalHitsContributions             edm4hep::CaloHitContribution
ZDCHcalHits                          edm4hep::SimCalorimeterHit
ZDCHcalHitsContributions             edm4hep::CaloHitContribution
~~~
