

| title                                             | teaching | exercises | questions                                                                        | objectives                                                                                                                                                                         | keypoints                                                                                                                                                                |
|---------------------------------------------------|----------|-----------|----------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Creating a plugin to make custom histograms/trees | 15       | 20        | Why should a I make a custom plugin? <br/><br/> How do I create a custom plugin? | Understand when one should make a plugin and when they should just use a ROOT macro. <br/><br/>Understand how to create a new, stand-alone plugin with your own custom histograms. | Plugins can be used to generate custom histograms by attaching directly to the reconstruction process. <br/><br/> Plugins can be used for monitoring or custom analysis. |



Plugins are the basic building blocks when it comes to analyzing data.  They request objects and perform actions, such as making histograms, or writing out certain objects to files.  When your plugin requests objects (e.g. clusters) the factory responsible for the requested object is loaded and run (We will dive into factories in the next exciting episode of how to use JANA).  When running EICrecon you will configure it to use some number of plugins (each potentially with their own set of configuration parameters). Now, let us begin constructing a new plugin.

To do this we will use the eicmkplugin.py script that comes with EICrecon.  This utility should be your "go-to" for jumpstarting your work with EICrecon/JANA when it comes to data. To put eicmkplugin.py in your path, you can do the following:
~~~
source EICrecon/bin/eicrecon-this.sh
~~~

The eickmkplugin script can be called simply by typing: "eicmkplugin.py" followed by the name of the plugin. Let's begin by calling: 
~~~
eicmkplugin.py  myFirstPlugin 
~~~

You should now have terminal output that looks like this:
~~~
Writing myFirstPlugin/CMakeLists.txt ...
Writing myFirstPlugin/myFirstPluginProcessor.h ...
Writing myFirstPlugin/myFirstPluginProcessor.cc ...

Created plugin myFirstPlugin.
Build with:

  cmake -S myFirstPlugin -B myFirstPlugin/build
  cmake --build myFirstPlugin/build --target install
~~~
There should now exist a new folder labeled "myPlugin". That directory contains 2 files: a CMakelists.txt file (needed for compiling our new plugin) and the source code for the plugin itself. 

Inside the source code for your plugin is a fairly simple class.  The private data members should contain the necessary variables to successfully run your plugin;  this will likely include any histograms, canvases, fits or other accoutrement. The public section contains the required Constructor, Init, Process, and Finish functions.  In init we get the application, as well as initialize any variables or histograms, etc.  The Process function typically gets objects from the event and does something with them (e.g. fill the histogram of cluster energy). And finally Finish is called where we clean up and do final things before ending the run of our plugin.

Before we compile our plugins we need to tell JANA about where the plugins will be found.  Start by setting your EICrecon_MY environment variable to a directory where you have write permission. The build instructions will install the plugin to that directory. When eicrecon is run, it will also look for plugins in the $EICrecon_MY directory and the EICrecon build you are using. This step is easy to overlook but necessary for the plugin to be found once compiled. Let's do this now before we forget:

~~~
mkdir EICrecon_MY
export EICrecon_MY=${PWD}/EICrecon_MY
~~~
 
To compile your plugin, let's follow the guidance given and type:
~~~
cmake -S myFirstPlugin -B myFirstPlugin/build
cmake --build myFirstPlugin/build --target install
~~~

You can test plugin installed and can load correctly by runnign eicrecon with it: 
~~~
eicrecon -Pplugins=myFirstPlugin,JTest -Pjana:nevents=10
~~~

The second plugin, JTest, just supplies dummy events, ensuring your plugin is properly compiled and found. To generate your first histograms, let's edit the myFirstPluginProcessor.cc and myFirstPluginProcessor.h files (located in the myFirstPlugin directory). Start by modifying myFirstPluginProcessor.h.  In the end it should look similar to the one below: 

```c++
#include <JANA/JEventProcessorSequentialRoot.h>
#include <TH2D.h>
#include <TFile.h>

#include <edm4hep/SimCalorimeterHit.h>

class myFirstPluginProcessor: public JEventProcessorSequentialRoot {
private:

    // Data objects we will need from JANA e.g.
    PrefetchT<edm4hep::SimCalorimeterHit> rawhits   = {this, "EcalBarrelHits"};

    // Declare histogram and tree pointers here. e.g.
    TH1D* hEraw = nullptr;

public:
    myFirstPluginProcessor() { SetTypeName(NAME_OF_THIS); }
    
    void InitWithGlobalRootLock() override;
    void ProcessSequential(const std::shared_ptr<const JEvent>& event) override;
    void FinishWithGlobalRootLock() override;
};
```
Next, edit the myFirstPluginProcessor.cc file to the following:
```c++
#include "myFirstPluginProcessor.h"
#include <services/rootfile/RootFile_service.h>

// The following just makes this a JANA plugin
extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new myFirstPluginProcessor);
    }
}

//-------------------------------------------
// InitWithGlobalRootLock
//-------------------------------------------
void myFirstPluginProcessor::InitWithGlobalRootLock(){

    // This ensures the histograms created here show up in the same root file as
    // other plugins attached to the process. Place them in dedicated directory
    // to avoid name conflicts.
    auto rootfile_svc = GetApplication()->GetService<RootFile_service>();
    auto rootfile = rootfile_svc->GetHistFile();
    rootfile->mkdir("myFirstPlugin")->cd();

    hEraw  = new TH1D("Eraw",  "BEMC hit energy (raw)",  100, 0, 0.075);
}

//-------------------------------------------
// ProcessSequential
//-------------------------------------------
void myFirstPluginProcessor::ProcessSequential(const std::shared_ptr<const JEvent>& event) {

     for( auto hit : rawhits() ) hEraw->Fill(  hit->getEnergy() );
}

//-------------------------------------------
// FinishWithGlobalRootLock
//-------------------------------------------
void myFirstPluginProcessor::FinishWithGlobalRootLock() {

    // Do any final calculations here.
}
```
Before we continue, stop for a moment and remember that plugins are compiled objects.  Thus it is imperative we rebuild our plugin after making any changes.  To do this, we can simply run the same commands we used to build the plugin in the first place:

~~~
cmake -S myFirstPlugin -B myFirstPlugin/build
cmake --build myFirstPlugin/build --target install
~~~

You can test the plugin using the following simulated data file:

~~~bash
wget https://eicaidata.s3.amazonaws.com/2022-09-26_ncdis10x100_minq2-1_200ev.edm4hep.root
eicrecon -Pplugins=myFirstPlugin 2022-09-26_ncdis10x100_minq2-1_200ev.edm4hep.root
~~~

You should now have a root file, eicrecon.root, with a single directory: "myFirstPlugin" containing the resulting hEraw histogram.


_____________________________________________________________________________________________________________

As exercises try (make sure you rebuild everytime you change your plugin):

1) Plot the X,Y positions of all the hits.  

2) Repeat only for hits with energy greater than 0.005 GeV.  

3) Try to plot similar histograms from the EcalEndcapN.

Feel free to play around with other objects and their properties (hint: when you ran eicrecon, you should have seen a list of all the objects that were available to you.  You can also see this list by typing: eicrecon -Pplugins=myFirstPlugin -Pjana:nevents=0)

Note: very shortly you will be adding a factory.  After you do come back to this plugin and access your newly created objects


{% include links.md %}

