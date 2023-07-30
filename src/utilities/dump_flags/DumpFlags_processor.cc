#include "DumpFlags_processor.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <fmt/core.h>


using namespace fmt;

//------------------
// DefaultFlags_processor (Constructor)
//------------------
DumpFlags_processor::DumpFlags_processor(JApplication *app) :
	JEventProcessor(app)
{
}

//------------------
// Init
//------------------
void DumpFlags_processor::Init()
{
	// Ask service locator a file to write to

    auto app = GetApplication();
    app->SetDefaultParameter("dump_flags:python", m_python_file_name, "If not empty, a python file to generate");
    app->SetDefaultParameter("dump_flags:markdown", m_markdown_file_name, "If not empty, a markdown file to generate");
    app->SetDefaultParameter("dump_flags:json", m_json_file_name, "If not empty, a json file to generate");
    app->SetDefaultParameter("dump_flags:screen", m_print_to_screen, "If not empty, print summary to screen at end of job");


    InitLogger(app, "dump_flags", "info");
}


//------------------
// Process
//------------------
void DumpFlags_processor::Process(const std::shared_ptr<const JEvent>& event)
{

}


//------------------
// Finish
//------------------
void DumpFlags_processor::Finish()
{
    auto pm = GetApplication()->GetJParameterManager();

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

    // Found longest values?
    spdlog::info("max_name_len={} max_default_val_len={}", max_name_len, max_default_val_len);

    // Start generating
    std::string python_content ="# format [ (flag_name, default_val, description), ...]\neicrecon_flags=[\n";
    std::string json_content = "[\n";

    // iterate over parameters
    size_t line_num = 0;
    for(auto [name,param]: pm->GetAllParameters())
    {
        // form python content string
        std::string python_escaped_descr = param->GetDescription();
        std::replace(python_escaped_descr.begin(), python_escaped_descr.end(), '\'', '`');
        python_content += fmt::format("    ({:{}} {:{}} '{}'),\n",
                                      fmt::format("'{}',", param->GetKey()),
                                          max_name_len + 3,
                                      fmt::format("'{}',", param->GetDefault()),
                                          max_default_val_len + 3,
                                      python_escaped_descr
                                          );

        // form json content string
        std::string json_escaped_descr = param->GetDescription();
        std::replace(json_escaped_descr.begin(), json_escaped_descr.end(), '"', '\'');
        json_content += fmt::format("    {}[\"{}\", \"{}\", \"{}\", \"{}\"]\n",
                                    line_num++==0?' ': ',',
                                    param->GetKey(),
                                    param->GetValue(),
                                    param->GetDefault(),
                                    json_escaped_descr);

        // Print on screen
        if( m_print_to_screen ) fmt::print("    {:{}} : {}\n", param->GetKey(), max_name_len + 3, param->GetValue());
    }

    // Finalizing
    python_content+="]\n\n";
    json_content+="]\n\n";

    // Save python file
    if(!m_python_file_name.empty()) {
        try{
            std::ofstream ofs(m_python_file_name);
            ofs << python_content;
            m_log->info("Created python file with flags: '{}'", m_python_file_name);
        }
        catch(std::exception ex) {
            m_log->error("Can't open file '{}' for write", m_python_file_name);    // TODO personal logger
            throw JException(ex.what());
        }
    }

    // Save json file
    if(!m_json_file_name.empty()) {
        try{
            std::ofstream ofs(m_json_file_name);
            ofs << json_content;
            m_log->info("Created json file with flags: '{}'", m_json_file_name);
            m_log->info("Json records format is: [name, value, default-value, comment]", m_json_file_name);
        }
        catch(std::exception ex) {
            m_log->error("Can't open file '{}' for write", m_json_file_name);    // TODO personal logger
            throw JException(ex.what());
        }
    }

    // Save JANA simple key-value file
    if(!m_janaconfig_file_name.empty()) {
        try{
            pm->WriteConfigFile(m_janaconfig_file_name);
        }
        catch(std::exception ex) {
            m_log->error("Can't open file '{}' for write", m_janaconfig_file_name);    // TODO personal logger
            throw JException(ex.what());
        }
    }
}
