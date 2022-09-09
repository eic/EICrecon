
## podio JEventSource Overview

This is the podio/edm4hep/edm4eic JANA plugin. This plugin is automatically
used by _eicrecon_ to read/write podio files. It may also be used with any
jana executable (including the generic _jana_ executable that comes with
JANA.) To use it with eicrecon to read in a podio file, you don't need to
do anything other than provide a podio file(s) on the command line:

~~~
eicrecon infile.root [infile2.root [infile3.root ...]]
~~~

To write to an output file, set the _podio:output_file_ configuration
parameter to the name of the output file. 

~~~
eicrecon -Pplugins=podio infile.root -Ppodio:output_file=outfile.root
~~~
_n.b. if you set the output file name to "1" it will use the name "podio_output.root"_

### Finding available collections
For _eicrecon_, there are direct command line options to list all available
object types/names which includes those in the input file. The podio plugin
itself also provides a mechanism to list only what is in the podio file.
To see a list of collections and their types contained in a file do this:

~~~
eicrecon -Ppodio:print_type_table=1 infile.root
...
Available Collections

Collection Name                      Data Type
-----------------------------------  -----------------------------------
MCParticles                          vector<edm4hep::MCParticleData>
EventHeader                          vector<edm4hep::EventHeaderData>
DIRCBarHits                          vector<edm4hep::SimTrackerHitData>
EcalEndcapNHits                      vector<edm4hep::SimCalorimeterHitData>
EcalEndcapNHitsContributions         vector<edm4hep::CaloHitContributionData>
EcalEndcapPHits                      vector<edm4hep::SimCalorimeterHitData>
EcalEndcapPHitsContributions         vector<edm4hep::CaloHitContributionData>
VertexBarrelHits                     vector<edm4hep::SimTrackerHitData>
DRICHHits                            vector<edm4hep::SimTrackerHitData>
TrackerBarrelHits                    vector<edm4hep::SimTrackerHitData>
TrackerEndcapHits1                   vector<edm4hep::SimTrackerHitData>
TrackerEndcapHits2                   vector<edm4hep::SimTrackerHitData>
MPGDTrackerBarrelHits1               vector<edm4hep::SimTrackerHitData>
TrackerEndcapHits3                   vector<edm4hep::SimTrackerHitData>
TrackerEndcapHits4                   vector<edm4hep::SimTrackerHitData>
MPGDTrackerBarrelHits2               vector<edm4hep::SimTrackerHitData>
TrackerEndcapHits5                   vector<edm4hep::SimTrackerHitData>
TrackerEndcapHits6                   vector<edm4hep::SimTrackerHitData>
GEMTrackerEndcapHits1                vector<edm4hep::SimTrackerHitData>
GEMTrackerEndcapHits2                vector<edm4hep::SimTrackerHitData>
GEMTrackerEndcapHits3                vector<edm4hep::SimTrackerHitData>
MRICHHits                            vector<edm4hep::SimTrackerHitData>
TOFBarrelHits                        vector<edm4hep::SimTrackerHitData>
EcalBarrelHits                       vector<edm4hep::SimCalorimeterHitData>
EcalBarrelHitsContributions          vector<edm4hep::CaloHitContributionData>
HcalBarrelHits                       vector<edm4hep::SimCalorimeterHitData>
HcalBarrelHitsContributions          vector<edm4hep::CaloHitContributionData>
HcalEndcapPHits                      vector<edm4hep::SimCalorimeterHitData>
HcalEndcapPHitsContributions         vector<edm4hep::CaloHitContributionData>
HcalEndcapNHits                      vector<edm4hep::SimCalorimeterHitData>
HcalEndcapNHitsContributions         vector<edm4hep::CaloHitContributionData>
B0PreshowerHits                      vector<edm4hep::SimTrackerHitData>
B0TrackerHits                        vector<edm4hep::SimTrackerHitData>
ZDCEcalHits                          vector<edm4hep::SimCalorimeterHitData>
ZDCEcalHitsContributions             vector<edm4hep::CaloHitContributionData>
ForwardOffMTrackerHits1              vector<edm4hep::SimTrackerHitData>
ForwardOffMTrackerHits2              vector<edm4hep::SimTrackerHitData>
ForwardOffMTrackerHits3              vector<edm4hep::SimTrackerHitData>
ForwardOffMTrackerHits4              vector<edm4hep::SimTrackerHitData>
ZDCHcalHits                          vector<edm4hep::SimCalorimeterHitData>
ZDCHcalHitsContributions             vector<edm4hep::CaloHitContributionData>
ForwardRomanPotHits1                 vector<edm4hep::SimTrackerHitData>
ForwardRomanPotHits2                 vector<edm4hep::SimTrackerHitData>
TaggerTracker1Hits                   vector<edm4hep::SimTrackerHitData>
TaggerCalorimeter1Hits               vector<edm4hep::SimCalorimeterHitData>
TaggerCalorimeter1HitsContributions  vector<edm4hep::CaloHitContributionData>
TaggerTracker2Hits                   vector<edm4hep::SimTrackerHitData>
TaggerCalorimeter2Hits               vector<edm4hep::SimCalorimeterHitData>
TaggerCalorimeter2HitsContributions  vector<edm4hep::CaloHitContributionData>
...
~~~

### Include/exclude collection lists for reading and writing
You may speed up how fast events are read in if you are only interested in certain collections
by configuring the include and exclude lists. For example if you are only interested in the
_MCParticles_ collection then do this:
~~~
eicrecon -Ppodio:input_include_collections=MCParticles infile.root 
~~~

You may also specify multiple collections using a comma separated list:
~~~
eicrecon -Ppodio:input_include_collections=MCParticles,EcalEndcapNHits infile.root 
~~~

Specify a list of collections to exclude like this:
~~~
eicrecon -Ppodio:input_exclude_collections=MCParticles,EcalEndcapNHits infile.root 
~~~

You may specify both an include list and an exclude list.


Similar to the input, you may also specify which collections to write out using the
_podio:output_include_collections_ and _podio:output_exclude_collections_ configuration
parameters. 

### Testing 
There may be certain instances where you would like to test an infinite stream of events, but
have a limited number of events in your root file. The _podio:run_forever_ flag will cause
the file to be repeatedly cycled over. For example:
~~~
eicrecon -Ppodio:run_forever=1 infile.root
~~~
Note that with this option set, only the first file will be read repeatedly. Any additional
files given on the command line will be ignored.

### Extra copy
One my specify that an additional copy of the output root file be made at the very
end of processing. The second file will have the same name as the first, but the
directory for this second copy may be specified like this:
~~~
eicrecon -Ppodio:output_file=myfile1.root -Ppodio:output_file_copy_dir=/path/to/copydir infile.root 
~~~
The above will result in a file _myfile1.root_ in the local directory and another copy
at _/path/to/copydir/myfile1.root_ .

### Merging in background events
One may specify a background event file that will have 1 or more events read and
merged into the primary event as it is read in. This is controlled by the
_podio:background_filename_ and _podio:num_background_events_ configuration
parameters.

Example: The command below will read in the primary (signal) events from _inputfile.root_
and for each signal event, it will add objects from 3 events from the file _background.root_
file.
~~~
eicrecon inputfile.root -Ppodio:background_filename=bacground.root -Ppodio:num_background_events=3
~~~

*NOTES:*

* The objects from the background events file will not be written to the output file (if
specified). Only objects from the primary input file and reconstructed values will be.
(This may change in the near future.)
* The background events will be recycled as needed so that the number of events in the
background file may be smaller than the number of events in the primary input file.
* This is implemented by opening _num_background_events_ instances of the background
events file. This not terribly efficient and will work fine up to about
_num_background_events_ <= 10. For values much larger than that, you 

### Techincal notes


* This uses a code generator to generate some routines that can take a class name 
in the form of a string and then call a templated function which can then use
that class. This is only possible for the limited number of data types defined
in the edm4hep data model. The code generation is done based on filenames it
finds matching the pattern _${EDM4HEP_ROOT}/include/edm4hep/*Collection.h_ . Thus,
if the model is updated, this will need to be recompiled, but it should automatically
find the classes defined in the updated model.


 




