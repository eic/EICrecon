//
// This is a JANA event source that uses PODIO to read from a ROOT
// file created by the "test/write" program that comes with PODIO.
// It only reads 3 data types from the file, but puts those objects
// into the JANA event for use by the rest of the framework.
//
// n.b. JANA uses copies of const pointers to the objects PODIO owns
// so memory usage etc. is optimized.
//
// n.b. This is currently NOT thread safe. This is due to PODIO using
// TTree underneath so reading a second event in will overwrite the
// current one (and thus, invalidate the pointers). A deeper
// understanding of PODIO is needed to fix this.

#include "JEventSourcePODIO.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

// podio specific includes
#include "podio/EventStore.h"
#include "podio/IReader.h"
#include "podio/UserDataCollection.h"
#include "podio/podioVersion.h"

//#include "datamodel/ExampleClusterCollection.h"
//#include "datamodel/ExampleHitCollection.h"
//#include "datamodel/ExampleMCCollection.h"

#include "datamodel/BasicParticleCollection.h"
#include "datamodel/CalorimeterHitCollection.h"
#include "datamodel/ClusterCollection.h"
#include "datamodel/InclusiveKinematicsCollection.h"
#include "datamodel/MCRecoClusterParticleAssociationCollection.h"
#include "datamodel/MCRecoParticleAssociationCollection.h"
#include "datamodel/MCRecoTrackParticleAssociationCollection.h"
#include "datamodel/MCRecoVertexParticleAssociationCollection.h"
#include "datamodel/PMTHitCollection.h"
#include "datamodel/ParticleIDCollection.h"
#include "datamodel/ProtoClusterCollection.h"
#include "datamodel/RawCalorimeterHitCollection.h"
#include "datamodel/RawPMTHitCollection.h"
#include "datamodel/RawTrackerHitCollection.h"
#include "datamodel/ReconstructedParticleCollection.h"
#include "datamodel/RingImageCollection.h"
#include "datamodel/TrackCollection.h"
#include "datamodel/TrackParametersCollection.h"
#include "datamodel/TrackSegmentCollection.h"
#include "datamodel/TrackerHitCollection.h"
#include "datamodel/TrajectoryCollection.h"
#include "datamodel/VertexCollection.h"
using namespace eicd;

// Make this a JANA plugin
extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->Add(new JEventSourceGeneratorT<JEventSourcePODIO>());
}
}

JEventSourcePODIO::JEventSourcePODIO(std::string resource_name, JApplication* app) : JEventSource(resource_name, app) {
    SetTypeName(NAME_OF_THIS); // Provide JANA with class name
}

void JEventSourcePODIO::Open() {

    // Open is called exactly once when processing begins.
	
    // Allow user to specify to recycle events forever
    GetApplication()->SetDefaultParameter("PODIO:RUN_FOREVER", run_forever, "set to true to recycle through events continuously");

    // Have PODIO reader open file and get the number of events from it.
    std::string filename = GetResourceName();
    reader.openFile( filename );
    if (reader.currentFileVersion() != podio::version::build_version) {
         _DBG_<<"Mismatch in PODIO versions! " << reader.currentFileVersion() << " != " << podio::version::build_version << std::endl;
        exit(-1);
    }
    Nevents_in_file = reader.getEntries();
    jout << "Opened PODIO file \""<< filename <<"\" with " << Nevents_in_file << " events" << std::endl;

    // Tell PODIO event store where to get its data from
    store.setReader(&reader);
}

void JEventSourcePODIO::GetEvent(std::shared_ptr <JEvent> event) {

    /// Calls to GetEvent are synchronized with each other, which means they can
    /// read and write state on the JEventSource without causing race conditions.
    
    // Check if we have exhausted events from file
    if( Nevents_read >= Nevents_in_file ) {
        if( run_forever ){
            Nevents_read = 0;
        }else{
            reader.closeFile();
            throw RETURN_STATUS::kNO_MORE_EVENTS;
        }
    }
	
    // This tells PODIO to free up the memory/caches used for the
    // collections and MetaData left from the last event.
    store.clear();
	
    // Tell PODIO which event to read into the store on the next calls to
    // getEventMetaData() and get<>() etc... below
    reader.goToEvent( Nevents_read++ );

    // For the example.root file generated in the PODIO example, there is one
    // meta data value for each type: string, float, int. These have names:
    // UserEventName, UserEventWeight, and SomeVectorData respectively.
    // We read it here for demonstration, but don't use it.
    const auto& evtMD = store.getEventMetaData();

    // We would normally want to obtain these from the metadata of the event/run
    event->SetEventNumber(Nevents_read);
    event->SetRunNumber(33);

    // The following reads the data for each of the collections into memory and
    // then copies the pointers into the JEvent so they can easily be used
    // by JANA algorithms. Ownership of the objects still resides with the
    // PODIO store. See the template definition in JEventSourcePODIO.h.
//    GetPODIOData<eicd::ExampleCluster, ExampleClusterCollection>("clusters"   , event);
//    GetPODIOData<eicd::ExampleHit,     ExampleHitCollection    >("hits"       , event);
//    GetPODIOData<eicd::ExampleMC,      ExampleMCCollection     >("mcparticles", event);

    GetPODIOData<BasicParticle,  BasicParticleCollection>("basicParticles", event);
    GetPODIOData<CalorimeterHit,  CalorimeterHitCollection>("calorimeterHits", event);
    GetPODIOData<Cluster,  ClusterCollection>("clusters", event);
    GetPODIOData<InclusiveKinematics,  InclusiveKinematicsCollection>("inclusiveKinematics", event);
    GetPODIOData<MCRecoClusterParticleAssociation,  MCRecoClusterParticleAssociationCollection>("MCRecoClusterParticleAssociation", event);
    GetPODIOData<MCRecoParticleAssociation,  MCRecoParticleAssociationCollection>("MCRecoParticleAssociation", event);
    GetPODIOData<MCRecoTrackParticleAssociation,  MCRecoTrackParticleAssociationCollection>("MCRecoTrackParticleAssociation", event);
    GetPODIOData<MCRecoVertexParticleAssociation,  MCRecoVertexParticleAssociationCollection>("MCRecoVertexParticleAssociation", event);
    GetPODIOData<PMTHit,  PMTHitCollection>("PMTHit", event);
    GetPODIOData<ParticleID,  ParticleIDCollection>("ParticleID", event);
    GetPODIOData<ProtoCluster,  ProtoClusterCollection>("ProtoCluster", event);
    GetPODIOData<RawCalorimeterHit,  RawCalorimeterHitCollection>("RawCalorimeter", event);
    GetPODIOData<RawPMTHit,  RawPMTHitCollection>("RawPMTHit", event);
    GetPODIOData<RawTrackerHit,  RawTrackerHitCollection>("RawTrackerHit", event);
    GetPODIOData<ReconstructedParticle,  ReconstructedParticleCollection>("ReconstructedParticle", event);
    GetPODIOData<RingImage,  RingImageCollection>("RingImage", event);
    GetPODIOData<Track,  TrackCollection>("Tracks", event);
    GetPODIOData<TrackParameters,  TrackParametersCollection>("TrackParameters", event);
    GetPODIOData<TrackSegment,  TrackSegmentCollection>("TrackSegment", event);
    GetPODIOData<TrackerHit,  TrackerHitCollection>("TrackerHit", event);
    GetPODIOData<Trajectory,  TrajectoryCollection>("Trajectory", event);
    GetPODIOData<Vertex,  VertexCollection>("Vertex", event);
	

}

std::string JEventSourcePODIO::GetDescription() {

    /// GetDescription() helps JANA explain to the user what is going on
    return "PODIO root file (example)";
}

template <>
double JEventSourceGeneratorT<JEventSourcePODIO>::CheckOpenable(std::string resource_name) {

    // If source is a root file, given minimal probability of success so we're chosen
    // only if no other ROOT file sources exist.
    return (resource_name.find(".root") != std::string::npos ) ? 0.01 : 0.0;
}
