---
title: "Creating or modifying a JANA factory in order to implement a reconstruction algorithm"
teaching: 15
exercises: 20
questions:
- "How to write a reconstruction algorithm in EICrecon?"
objectives:
- "Learn how to create a new factory in EICrecon that supplies a reconstruction algorithm for all to use."
- "Understand the directory structure for where the factory should be placed in the source tree."
- "Understand how to use a generic algorithm in a JANA factory."
keypoints:
- "Create a factory for reconstructing single subdetector data or for global reconstruction."
---
The JANA Framework is based on a series of factories.  Ultimately, plugins request widgets needed for analysis and via that request the factories JANA knows are spun up, as needed, to produce the desired widgets.
So, you want to make a new widget? We need to make a new factory.  This factory will take in some set of input(s) and produce some set of output(s) (our shiny new widgets).

To create a new factory we will use jana_generate.

From the eic shell: "jana-generate.py JFactory myWidget"

this will produce 2 files: JFactory_myWidget.h/.cc and a 3rd file: myWidget.h

myWidget is a simple struct which holds the data for my widget. JFactory_myWidget.h holds our new factory's declarations.  Notice the class is public JFactoryT, it is going to produce the templated object, which in this case is "myWidget".  Perhaps your widgets come in different colors or require specific calibrations to produce; you can put any needed variables for the production of the widgets where it says "Insert member variables here". Note,  the variables held in myWidget do not need to e redundantly placed here. As public members JFactory_myWidget we have the constructor, an initialiation function, a function which is called when the run number changes and the main Process function.  You may place helper functions' declarations here as well. We will go into more detail of each function of the factory momentarily.

Looking at JFactory_myWidget.cc we can see/fill out the declared function.  The constructor can do many things (e.g. initialize variables). It should, at the very least, set a tag.  This tag is used to access the objects created by the factory (which invokes the factory).

The init function is called before any event is processed by the factory.  Here you would get needed parameters and services, setting an specific factory flags.  Any other "preprocessing" can be done here as well.

The changeRun function is run before when a new run number is seen.  This is useful for Run dependent quatities, such as calirations.  Any other run-dependent action may be taken as well; such as creating a different widget color depending on run number (if this were a plugin you could plot the energy deposited for each runnumber).

Finally, we arrive at the meat of factory the Process function.  Here you ask the event for some ojects to form an input.  Then you do whatever computation you desire.  Then you store the results back to JANA.  

Now, once compiled plugins can begin requesting "myWidget"s.


{% include links.md %}
