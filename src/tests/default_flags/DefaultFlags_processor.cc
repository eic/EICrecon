#include "DefaultFlags_processor.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <fmt/core.h>

#include <edm4hep/SimCalorimeterHit.h>
#include <edm4hep/MCParticle.h>

#include <TDirectory.h>
#include <TCanvas.h>
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <Math/LorentzVector.h>
#include <Math/GenVector/PxPyPzM4D.h>
#include <services/rootfile/RootFile_service.h>


using namespace fmt;

//------------------
// DefaultFlags_processor (Constructor)
//------------------
DefaultFlags_processor::DefaultFlags_processor(JApplication *app) :
	JEventProcessor(app)
{
}

//------------------
// Init
//------------------
void DefaultFlags_processor::Init()
{
	// Ask service locator a file to write to

}


//------------------
// Process
//------------------
void DefaultFlags_processor::Process(const std::shared_ptr<const JEvent>& event)
{

}


//------------------
// Finish
//------------------
void DefaultFlags_processor::Finish()
{
    auto pm = GetApplication()->GetJParameterManager();

    for(auto [name,param]: pm->GetAllParameters())
    {
        fmt::print("{:<40} {} {} {}\n", name, param->GetKey(), param->GetDefault(), param->GetDescription());
    }


}

