---
title: "Creating a JANA plugin to make custom histograms/trees"
teaching: 15
exercises: 20
questions:
- "Why should a I make a custom plugin?"
- "How do I create a custom JANA plugin?"
objectives:
- "Understand when one should make a JANA plugin and when they should just use a ROOT macro."
- "Understand how to create a new, stand-alone plugin with your own custom histograms."
keypoints:
- "JANA plugins can be used to generate custom histograms by attaching directly to the reconstruction process."
- "Plugins can be used for monitoring or custom analysis."
---
Plugins are the basic building blocks when it comes to analyzing data.  They request objects and perform actions, such as making histograms, or writing out certain objects to files.  When your plugin requests objects (e.g. clusters) the factory responsible for the requested object is loaded and run (We will dive into factories in the next exciting episode of how to use JANA).  When running EICrecon you will configure it to use some number of plugins (each potentially with their own set of configuration parameters). Now, let us begin constructing a new plugin.

To do this we will use the jana-generate script that comes with EICrecon.  This utility should be your "go-to" for jumpstarting your work with EICrecon/JANA. 

The jana-generate script can be called simply by typing: "jana-generate.py" followed by the type of thing you want the code skeleton for and additional arguments as needed. Valid things to produce the code skeleton for are:
JObject
JEventSource
JEventProcessor
RootEventProcessor
JEventProcessorTest
JFactory
Plugin
Project

for this tutorial we will be focusing on Plugin (now) and JFactory (next episode).  Let's begin by calling: "jana-generate.py Plugin myPlugin".  After a moment there should now exist a new folder labeled "myPlugin". That directory contains 2 files: a CMakelists.txt file (needed for compiling our new plugin) and the source code for the plugin itself. 

Inside the source code for your plugin is a fairly simple class.  The private data members should contain the necessary variables to successfully run your plugin;  this will likely include any histograms, canvases, fits or other accoutrement. The public section contains the required Constructor, Init, Process, and Finish functions.  In the constructor we need to supply JANA the name of our plugin.  In init we get the application, as well as initialize any variables or histograms (etc etc).  The Process function typically gets objects from the event and does something with them (e.g. fill the histogram of cluster energy). And finally Finish is called where we clean up and do final things like writing the resulting root files.



{% include links.md %}

