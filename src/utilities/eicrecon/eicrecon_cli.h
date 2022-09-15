//
// Created by xmei on 9/7/22.
//

#ifndef EICRECON_EICRECON_CLI_H
#define EICRECON_EICRECON_CLI_H

#include <JANA/JApplication.h>

namespace jana {

    enum Flag {
        Unknown,
        ShowUsage,
        ShowVersion,
        ShowDefaultPlugins,
        ShowAvailablePlugins,
        ShowConfigs,
        LoadConfigs,
        DumpConfigs,
        Benchmark,
        ListFactories
    };

    struct UserOptions {
        /// Code representation of all user options.
        /// This lets us cleanly separate args parsing from execution.

        std::map<Flag, bool> flags;
        std::map<std::string, std::string> params;
        std::vector<std::string> eventSources;
        std::string load_config_file;
        std::string dump_config_file;
    };

    /// Read the user options from the command line and initialize @param options.
    /// If there are certain flags, mark them as true.
    /// Push the event source strings to @param options.eventSources.
    /// Push the parameter strings to @param options.params as key-value pairs.
    /// If the user option is to load or dump a config file, initialize @param options.load/dump_config_file
    UserOptions GetCliOptions(int nargs, char *argv[], bool expect_extra=true);

    /// If the user option contains print only flags, print the info ann return true; otherwise return false.
    /// The print only flags include: "-v", "-h", "-L", "--list_default_plugins", "--list_available_plugins".
    /// When these flags are effective, the application will exit immediately.
    bool HasPrintOnlyCliOptions(UserOptions& options, std::vector<std::string> const& default_plugins);

    void PrintUsage();
    void PrintVersion();

    /// List the @param default_plugins in a table.
    /// @param default_plugins is given at the top of the eicrecon.cc.
    void PrintDefaultPlugins(std::vector<std::string> const& default_plugins);

    /// List all the available plugins at @env_var $JANA_PLUGIN_PATH and @env_var $EICrecon_MY/plugins.
    void PrintAvailablePlugins(std::vector<std::string> const& default_plugins);

    void PrintFactories(JApplication* app);
    void PrintPodioCollections(JApplication* app);

    JApplication* CreateJApplication(UserOptions& options);
    int Execute(JApplication* app, UserOptions& options);

}

#endif //EICRECON_EICRECON_CLI_H
