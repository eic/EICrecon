# Visualizing Calllgraphs

## Overview
JANA2 has a built-in feature to help visualize the callgraph of a job.
This is most easily activated by adding the `janadot` plugin to the
job. It will automatically turn on JANA's call stack recording which
keeps track of the call dependencies between factories, processors, and
sources in the event. It also times how long it takes each factory
to run, integrating it over all calls so that one can see the relative
time spent in each factory.

The `janadot` plugin in EICrecon includes enhanced functionality to automatically
split large graphs into multiple smaller graphs for better processing by graphviz
and improved readability.

*Note that this requires JANA2 v2.0.8 or later and EICrecon v0.3.6 or
later.*

## How to run
To make an image of the call graph of a job, one just needs to add
the `janadot` plugin like this:
~~~bash
eicrecon -Pplugins=janadot sim_file.edm4hep.root
~~~

This will create a file at the end of the job called "jana.dot"
which contains the relevant information in a form that the
*dot* program can read. (*dot comes in the graphviz package
on most Linux distributions*). At the end of the job, a
message like the following is printed:

~~~bash
Opening output file "jana.dot"

Factory calling information written to "jana.dot". To create a graphic
from this, use the dot program. For example, to make a PDF file do the following:

   dot -Tpdf jana.dot -o jana.pdf

This should give you a file named "jana.pdf".
~~~

You can use dot to make either a PDF or PNG formatted file. To make a PNG file
you would run:

~~~bash
dot -Tpng jana.dot -o jana.png
~~~

### Graph Splitting for Large Call Graphs
When processing complex reconstructions with many algorithms, the resulting call graph can become too large for graphviz to handle efficiently. The janadot plugin automatically detects large graphs and splits them into multiple smaller graphs.

To control the splitting behavior, you can use these parameters:

~~~bash
# Enable/disable splitting (enabled by default)
eicrecon -Pplugins=janadot -Pjanadot:enable_splitting=false sim_file.edm4hep.root

# Control splitting thresholds
eicrecon -Pplugins=janadot \
   -Pjanadot:max_nodes_per_graph=30 \
   -Pjanadot:max_edges_per_graph=60 \
   sim_file.edm4hep.root

# Change splitting method (plugin, size, components, or type)
eicrecon -Pplugins=janadot -Pjanadot:split_criteria=components sim_file.edm4hep.root
~~~

#### Plugin-based Splitting (Default)
The default splitting method groups components by detector subsystem plugins, providing both detailed subsystem views and high-level architectural overview:

~~~bash
# Plugin-based splitting for system architecture analysis
eicrecon -Pplugins=janadot -Pjanadot:split_criteria=plugin sim_file.edm4hep.root
~~~

This generates:
- `jana.tracking.dot` - All tracking-related components
- `jana.ecal_barrel.dot` - ECAL barrel subsystem components
- `jana.hcal_endcap.dot` - HCAL endcap subsystem components
- `jana.dot` - Overall inter-plugin connection summary

#### Other Splitting Methods
When graphs are split using size, components, or type methods, multiple files are created (`jana_part001.dot`, `jana_part002.dot`, etc.) along with an index file (`jana_index.txt`) that explains how to process them.

### Running for a single detector
By default `eicrecon` activates the full reconstruction. This will
result in a very busy callgraph that can be hard to read if you
are only interested in one detector. (See [here](https://eic.github.io/EICrecon/howto/callgraphs/all.png).)
To activate for a specific factory, you can specify the relevant output
collection using the `podio:output_collections` parameter.
Here is an example for the *EcalEndcapN*:

~~~bash
eicrecon -Pplugins=janadot \
   -Ppodio:output_collections=EcalEndcapNMergedClusters \
   sim_file.edm4hep.root
~~~

The above will produce something like the following:
![EcalEndcapNMergedClusters](callgraphs/EcalEndcapNMergedClusters.png)

In the plot, the oval at the top is the JEventProcessor making the
initial request for objects. (This is where the *podio:output_collections*
parameter is used). The rectangles in the middle represent factories (i.e. algorithms).
The green trapezoid at the bottom is the object read from the input file.
This is representative of the flow JANA implements. Namely, the requests for
data objects flow from top to bottom and the data objects themselves are
actually created in the order going from bottom to top.

The arrows point in the direction of the request and the numbers next to
them indicate 1.) how many times that request was made, 2.) the total amount of time
spent satisfying the request, and 3.) the fraction of the overall job time
that represents. In this example, 100 events were processed and so the request
was made 100 times at each stage. Similarly, the numbers in the boxes indicate
the total time spent in that factory, integrated over all calls, and the total
job fraction it represents.

Note that the only factory specified in the above example was for the
`EcalEndcapNMergedClusters` collection. All of the other factories shown
were activated  in order to satisfy the request for the `EcalEndcapNMergedClusters`
objects. If instead we would have specified only the `EcalEndcapNRecHits`, then
only the `EcalEndcapNRecHits` and `EcalEndcapNRawHits` factories would have shown
up in the plot.


## Examples
Below are some example images produced for 100 DIS events. The events were
simulated using the following on the JLab ifarm:

~~~bash
cp /volatile/eic/EPIC/MC_input/SIDIS/Lambda_ABCONV/hepmc_Lambda1_ab_hiAcc_18x275_00.hepmc.gz .
tar xzf hepmc_Lambda1_ab_hiAcc_18x275_00.hepmc.gz
ddsim --compactFile $DETECTOR_PATH/epic_arches.xml --numberOfEvents 100 --inputFiles hepmc_Lambda1_ab_hiAcc_18x275_00.hepmc --outputFile sim_hepmc3_Lambda1_ab_hiAcc_18x275_00.edm4hep.root
~~~

Note that in these examples a relatively small number of events was processed
and the startup time which included reading in the geometry from DD4Hep is
included. Thus, it is often the case that the factory indicating the largest
time is just the first place where the geometry was read in.

| Example images                                                                                 |
|------------------------------------------------------------------------------------------------|
| ![All Factories](callgraphs/all.png)                                                           |
| ![B0ECalClusters](callgraphs/B0ECalClusters.png)                                               |
| ![EcalBarrelSciGlassMergedTruthClusters](callgraphs/EcalBarrelSciGlassMergedTruthClusters.png) |
| ![EcalBarrelImagingMergedClusters](callgraphs/EcalBarrelImagingMergedClusters.png)             |
| ![EcalEndcapPMergedClusters](callgraphs/EcalEndcapPMergedClusters.png)                         |
| ![EcalEndcapNMergedClusters](callgraphs/EcalEndcapNMergedClusters.png)                         |
| ![ForwardRomanPotParticles](callgraphs/ForwardRomanPotParticles.png)                           |
| ![HcalBarrelClusters](callgraphs/HcalBarrelClusters.png)                                       |
| ![HcalEndcapPClusters](callgraphs/HcalEndcapPClusters.png)                                     |
| ![HcalEndcapPInsertClusters](callgraphs/HcalEndcapPInsertClusters.png)                         |
| ![HcalEndcapNClusters](callgraphs/HcalEndcapNClusters.png)                                     |
| ![ReconstructedChargedParticles](callgraphs/ReconstructedChargedParticles.png)                 |
| ![SmearedFarForwardParticles](callgraphs/SmearedFarForwardParticles.png)                       |
| ![ZDCEcalMergedClusters](callgraphs/ZDCEcalMergedClusters.png)                                 |
