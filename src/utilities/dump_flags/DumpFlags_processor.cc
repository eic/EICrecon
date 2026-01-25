#include "DumpFlags_processor.h"

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/JException.h>
#include <JANA/Services/JParameterManager.h>
#include <fmt/format.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <exception>
#include <fstream>
#include <map>
#include <memory>
#include <regex>
#include <utility>

using namespace fmt;

//------------------
// Init
//------------------
void DumpFlags_processor::Init() {
  // Ask service locator a file to write to

  auto* app = GetApplication();
  app->SetDefaultParameter("dump_flags:python", m_python_file_name,
                           "If not empty, a python file to generate");
  app->SetDefaultParameter("dump_flags:markdown", m_markdown_file_name,
                           "If not empty, a markdown file to generate");
  app->SetDefaultParameter("dump_flags:json", m_json_file_name,
                           "If not empty, a json file to generate");
  app->SetDefaultParameter("dump_flags:screen", m_print_to_screen,
                           "If not empty, print summary to screen at end of job");

  InitLogger(app, "dump_flags", level::info);
}

//------------------
// Process
//------------------
void DumpFlags_processor::Process(const std::shared_ptr<const JEvent>& /* event */) {}

static std::string json_escape(const std::string& str) {
  std::string res = str;
  std::regex newline_re("\n");
  res = std::regex_replace(res, newline_re, "\\n");
  std::regex quote_re("\"");
  res = std::regex_replace(res, quote_re, "\\\"");
  return res;
}
//------------------
// Finish
//------------------
void DumpFlags_processor::Finish() {
  auto* pm = GetApplication()->GetJParameterManager();

  // Find longest strings in names and values
  std::size_t max_name_len        = 0;
  std::size_t max_default_val_len = 0;
  for (auto [name, param] : pm->GetAllParameters()) {
    max_name_len = std::max(max_name_len, strlen(name.c_str()));

    max_default_val_len = std::max(max_default_val_len, strlen(param->GetDefault().c_str()));
  }

  // Found longest values?
  spdlog::info("max_name_len={} max_default_val_len={}", max_name_len, max_default_val_len);

  // Start generating
  std::string python_content =
      "# format [ (flag_name, default_val, description), ...]\neicrecon_flags=[\n";
  std::string json_content = "[\n";

  // iterate over parameters
  std::size_t line_num = 0;
  for (auto [name, param] : pm->GetAllParameters()) {
    // form python content string
    std::string python_escaped_descr = param->GetDescription();
    std::ranges::replace(python_escaped_descr, '\'', '`');
    python_content += fmt::format(
        "    ({:{}} {:{}} '{}'),\n", fmt::format("'{}',", param->GetKey()), max_name_len + 3,
        fmt::format("'{}',", param->GetDefault()), max_default_val_len + 3, python_escaped_descr);

    // form json content string
    json_content +=
        fmt::format("    {}[\"{}\", \"{}\", \"{}\", \"{}\"]\n", line_num++ == 0 ? ' ' : ',',
                    json_escape(param->GetKey()), json_escape(param->GetValue()),
                    json_escape(param->GetDefault()), json_escape(param->GetDescription()));

    // Print on screen
    if (m_print_to_screen) {
      fmt::print("    {:{}} : {}\n", param->GetKey(), max_name_len + 3, param->GetValue());
    }
  }

  // Finalizing
  python_content += "]\n\n";
  json_content += "]\n\n";

  // Save python file
  if (!m_python_file_name.empty()) {
    try {
      std::ofstream ofs(m_python_file_name);
      ofs << python_content;
      m_log->info("Created python file with flags: '{}'", m_python_file_name);
    } catch (std::exception& ex) {
      m_log->error("Can't open file '{}' for write", m_python_file_name); // TODO personal logger
      throw JException(ex.what());
    }
  }

  // Save json file
  if (!m_json_file_name.empty()) {
    try {
      std::ofstream ofs(m_json_file_name);
      ofs << json_content;
      m_log->info("Created json file with flags: '{}'", m_json_file_name);
      m_log->info("Json records format is: [name, value, default-value, comment]",
                  m_json_file_name);
    } catch (std::exception& ex) {
      m_log->error("Can't open file '{}' for write", m_json_file_name); // TODO personal logger
      throw JException(ex.what());
    }
  }

  // Save JANA simple key-value file
  if (!m_janaconfig_file_name.empty()) {
    try {
      pm->WriteConfigFile(m_janaconfig_file_name);
    } catch (std::exception& ex) {
      m_log->error("Can't open file '{}' for write",
                   m_janaconfig_file_name); // TODO personal logger
      throw JException(ex.what());
    }
  }
}
