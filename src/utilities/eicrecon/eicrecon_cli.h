//
// Created by xmei on 9/7/22.
//

#pragma once

#include <JANA/JApplicationFwd.h>
#include <map>
#include <string>
#include <vector>

#ifdef alignas
// FIXME may be removed when minimum version in CMakeLists.txt includes the PR below
#error JANA defines alignas macro; for patch see https://github.com/JeffersonLab/JANA2/pull/239
#endif

namespace jana {

enum Flag {
  Unknown,
  ShowUsage,
  ShowVersion,
  ShowJANAVersion,
  ShowDefaultPlugins,
  ShowAvailablePlugins,
  ShowConfigs,
  LoadConfigs,
  DumpConfigs,
  Benchmark,
  ListFactories,
  ListPluginFactories,
  PrintFactoryInfo
};

struct UserOptions {
  /// Code representation of all user options.
  /// This lets us cleanly separate args parsing from execution.

  std::map<Flag, bool> flags;
  std::map<std::string, std::string> params;
  std::vector<std::string> eventSources;
  std::string load_config_file;
  std::string dump_config_file;
  std::string plugin_name; // For --list-available-factories <plugin>
};

/// Read the user options from the command line and initialize @param options.
/// If there are certain flags, mark them as true.
/// Push the event source strings to @param options.eventSources.
/// Push the parameter strings to @param options.params as key-value pairs.
/// If the user option is to load or dump a config file, initialize @param options.load/dump_config_file
UserOptions GetCliOptions(int nargs, char* argv[], bool expect_extra = true);

/// If the user option contains print only flags, print the info ann return true; otherwise return false.
/// The print only flags include: "-v", "-h", "-L", "--list_default_plugins", "--list_available_plugins",
/// "--list-available-factories", "--print-factory-info".
/// When the info is shown, the application will exit immediately.
bool HasPrintOnlyCliOptions(UserOptions& options, std::vector<std::string> const& default_plugins);

void PrintUsage();
void PrintVersion();

/// List the @param default_plugins in a table.
/// @param default_plugins is given at the top of the eicrecon.cc.
void PrintDefaultPlugins(std::vector<std::string> const& default_plugins);

/// List all the available plugins at @env_var $JANA_PLUGIN_PATH and @env_var $EICrecon_MY/plugins.
/// @note Does not guarantee the effectiveness of the plugins.
/// @note The plugins can be override if they use the same name under different locations.
void PrintAvailablePlugins(std::vector<std::string> const& default_plugins);

/// Add the default plugins and the plugins at $EICrecon_MY/plugins to @param options.params.
/// It comes before creating the @class JApplication.
void AddAvailablePluginsToOptionParams(UserOptions& options,
                                       std::vector<std::string> const& default_plugins);

void AddDefaultPluginsToJApplication(JApplication* app,
                                     std::vector<std::string> const& default_plugins);

void PrintFactories(JApplication* app);
void PrintPodioCollections(JApplication* app);

/// List all factories for a specific plugin
void PrintPluginFactories(JApplication* app, const std::string& plugin_name);

/// Print detailed factory information including input/output collections
void PrintFactoryInfo(JApplication* app);

/// Copy the @param options params (from the cli or the config file) to a JParameterManager @var para_mgr.
/// Create an empty JApplication @var app.
/// Add the event sources got from the cli input to @var app, and then return.
/// @note The cli -Pkey=value pairs are not processed when the function returns. They are processed,
/// or, added to @var app at calling JApplication::Initialize().
JApplication* CreateJApplication(UserOptions& options);
int Execute(JApplication* app, UserOptions& options);

} // namespace jana
