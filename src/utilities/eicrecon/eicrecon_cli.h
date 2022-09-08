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

    void PrintUsage();
    void PrintUsageOptions();
    void PrintVersion();
    UserOptions ParseCommandLineOptions(int nargs, char *argv[], bool expect_extra=true);
    JApplication* CreateJApplication(UserOptions& options);
    int Execute(JApplication* app, UserOptions& options);
    void PrintFactories(JApplication* app);
    void PrintPodioCollections(JApplication* app);
}

#endif //EICRECON_EICRECON_CLI_H
