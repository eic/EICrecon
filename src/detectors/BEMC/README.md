
Here is an example of using a generic algorithm class with a JANA factory.

For the example, we implement the Juggler algorithm found here:

https://eicweb.phy.anl.gov/EIC/juggler/-/blob/master/JugDigi/src/components/CalorimeterHitDigi.cpp

This takes in objects of type _edm4hep::SimCalorimeterHit_ and produces
objects of type _edm4hep::RawCalorimeterHit_. The _CalorimeterHitDigi_ class
effectively knows nothing of JANA and could be used in other contexts.

There are two ways one could implement this in a JFactory:

1. Using multiple inheritance. This is what is done in the _JFactory_BEMCRawCalorimeterHit_ class.

2. Including the algorithm object as a data member. This is what is done in the
_JFactory_BEMCRawCalorimeterHit_utility_ class.

The main difference in the coding is that for option 2, one needs to dereference the algorithm
class object whenever referring to one of its members. Otherwise, they are line-for-line the same.

The _CalorimeterHitDigi_ class methods are not reentrant, This is not an issue
since only one thread will be executing the method of a given object at a time.
(The same method of multiple objects may be executed in parallel.) Thus,
inputs and outputs of the class are stored in data members of _CalorimeterHitDigi_.

Two things I don't like about this system:

1. The data objects created by the generic algorithm must be effectively cloned into
   _BEMRawCalorimeterHit_ objects.

2. The framework has no way to automatically delete the _edm4hep::RawCalorimeterHit_ objects
so we handle it manually in the _CalorimeterHitDigi_ class.


One other option I have included is defining the _SetJANAConfigParameters_ templated method
in the generic algorithm class _CalorimeterHitDigi_. This technically includes JANA code
in the class, but since it is contained in the template, the compiler will never complain
unless a template object is decalred, Thus, the _CalorimeterHitDigi_ class can be used
outside of the JANA framework. The pros/cons of doing this are:

pros:
- It places this code in the header close to the data member definitions
- It makes this code reusable for all calorimeters that use this algorithm

cons:
- It places JANA code in a generic algorithm which could be confusing if used
  outside of JANA
- It may be confusing to others trying to understand the code as to why this
  is a template




<hr>

Below is a list of the simulated collections and types for reference.

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
