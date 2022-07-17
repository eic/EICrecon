
#include "JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits.h"

#include <JANA/JEvent.h>

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// Please add the following lines to your InitPlugin or similar routine
// in order to register this factory with the system.
//
// #include "JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits.h"
//
//     app->Add( new JFactoryGeneratorT<JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits>() );
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 


//------------------------
// Constructor
//------------------------
JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits::JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits(){
    SetTag("EcalBarrelRawCalorimeterHits");
}

//------------------------
// Init
//------------------------
void JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits::Init() {
    auto app = GetApplication();
    
    /// Acquire any parameters
    // app->GetParameter("parameter_name", m_destination);
    
    /// Acquire any services
    // m_service = app->GetService<ServiceT>();
    
    /// Set any factory flags
    // SetFactoryFlag(JFactory_Flags_t::NOT_OBJECT_OWNER);
}

//------------------------
// ChangeRun
//------------------------
void JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    /// This is automatically run before Process, when a new run number is seen
    /// Usually we update our calibration constants by asking a JService
    /// to give us the latest data for this run number
    
    auto run_nr = event->GetRunNumber();
    // m_calibration = m_service->GetCalibrationsForRun(run_nr);
}

//------------------------
// Process
//------------------------
void JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits::Process(const std::shared_ptr<const JEvent> &event) {

    /// JFactories are local to a thread, so we are free to access and modify
    /// member variables here. However, be aware that events are _scattered_ to
    /// different JFactory instances, not _broadcast_: this means that JFactory 
    /// instances only see _some_ of the events. 
    
    /// Acquire inputs (This may recursively call other JFactories)
    // auto inputs = event->Get<...>();
    
    /// Do some computation
    
    /// Publish outputs
    // std::vector<RawCalorimeterHit*> results;
    // results.push_back(new RawCalorimeterHit(...));
    // Set(results);
}
