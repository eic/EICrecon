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
#include <extensions/string/StringHelpers.h>


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

    auto app = GetApplication();
    app->SetDefaultParameter("default_flags:python", m_python_file_name, "If not empty, a python file to generate");
    app->SetDefaultParameter("default_flags:markdown", m_markdown_file_name, "If not empty, a markdown file to generate");
    app->SetDefaultParameter("default_flags:json", m_json_file_name, "If not empty, a json file to generate");
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

    // Set to false all categories use flags before we go through flags
    std::map<std::string, bool> category_is_used;
    for(const auto &category: m_reco_prefixes) {
        category_is_used[category] = false;
    }

    std::string python_content = "flags=[\n";

    std::string all_python_content ="# format [ (flag_name, default_val, description), ...]\neicrecon_flags=[\n";

    // Find longest strings in names and values
    size_t max_name_len = 0;
    size_t max_default_val_len = 0;
    for(auto [name,param]: pm->GetAllParameters()) {
        if(max_name_len < strlen(name.c_str())) {
            max_name_len = strlen(name.c_str());
        }

        if(max_default_val_len < strlen(param->GetDefault().c_str())) {
            max_default_val_len = strlen(param->GetDefault().c_str());
        }
    }

    spdlog::info("max_name_len={} max_default_val_len={}", max_name_len, max_default_val_len);


    for(auto [name,param]: pm->GetAllParameters())
    {
        std::string escaped_descr = eicrecon::str::ReplaceAllCopy(param->GetDescription(), "'", "`");

        all_python_content += fmt::format("    ({:{}} {:{}} '{}'),\n",
                                          fmt::format("'{}',", param->GetKey()),
                                          max_name_len + 3,
                                          fmt::format("'{}',", param->GetDefault()),
                                          max_default_val_len + 3,
                                          escaped_descr
                                          );


        if (isReconstructionFlag(name)) { // pos=0 limits the search to the prefix

            // Is it the first flag in the category?
            std::string category = findCategory(name);
            if(!category.empty() && !category_is_used[category]) {

            }
            // s starts with prefix
            auto escaped_flag_name = fmt::format("'{}',", param->GetKey());


//            python_content += fmt::format("    {:{}}   # {:{}} '{}' '{}'\n",
//                                          escaped_flag_name, max_name_len+3,
//                                          param->GetDefault(),  max_default_val_len,
//                                          param->GetValue(),
//                                          param->GetDescription());
            fmt::print("{:<{}} - {} \n", name, max_name_len+1, param->GetDescription());
        }
//fmt::print("{:<40} {} {} {}\n", name, param->GetKey(), param->GetDefault(), param->GetDescription(), param->GetValue());
    }

    python_content+="]\n\n";
    all_python_content+="]\n\n";
    fmt::print(python_content);

    if(!m_python_file_name.empty()) {
        try{
            std::ofstream ofs(m_python_file_name);
            ofs << all_python_content;
            spdlog::info("Created python file with flags: '{}'", m_python_file_name);
        }
        catch(std::exception ex) {
            spdlog::error("Can't open file '{}' for write", m_python_file_name);    // TODO personal logger
            throw;
        }
    }
}

