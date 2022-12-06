//
// Created by xmei on 9/7/22.
//

#include "eicrecon_cli.h"
#include "print_info.h"
#include "parser.h"

#include <JANA/CLI/JVersion.h>
#include <JANA/CLI/JBenchmarker.h>
#include <JANA/CLI/JSignalHandler.h>

#include <JANA/Services/JComponentManager.h>

#include <set>
#include <iostream>
#include <string>
#include <filesystem>

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)

#ifndef EICRECON_APP_VERSION
#  define EICRECON_APP_VERSION Error
#endif

#define EICRECON_APP_VERSION_STR STR(EICRECON_APP_VERSION)


namespace jana {

    void PrintUsageOptions() {
        std::cout << "Options:" << std::endl;
        std::cout << "   -h   --help                  Display this message" << std::endl;
        std::cout << "   -v   --version               Display version information" << std::endl;
        std::cout << "   -j   --janaversion           Display JANA version information" << std::endl;
        std::cout << "   -c   --configs               Display configuration parameters" << std::endl;
        std::cout << "   -l   --loadconfigs <file>    Load configuration parameters from file" << std::endl;
        std::cout << "   -d   --dumpconfigs <file>    Dump configuration parameters to file" << std::endl;
        std::cout << "   -b   --benchmark             Run in benchmark mode" << std::endl;
        std::cout << "   -L   --list-factories        List all the factories without running" << std::endl;
        std::cout << "   -Pkey=value                  Specify a configuration parameter" << std::endl;
        std::cout << "   -Pplugin:param=value         Specify a parameter value for a plugin" << std::endl;
        std::cout << std::endl;

        std::cout << "   --list-default-plugins       List all the default plugins" << std::endl;
        std::cout << "   --list-available-plugins     List plugins at $JANA_PLUGIN_PATH and $EICrecon_MY" << std::endl;
        std::cout << std::endl << std::endl;
    }

    void PrintUsageExample() {

        std::cout << "Example:" << std::endl;
        std::cout << "    eicrecon -Pplugins=plugin1,plugin2,plugin3 -Pnthreads=8 infile.root" << std::endl;
        std::cout << "    eicrecon -Ppodio:print_type_table=1 infile.root" << std::endl << std::endl;
        std::cout << std::endl << std::endl;
    }

    void PrintUsage() {
        /// Prints jana.cc command-line options to stdout, for use by the CLI.
        /// This does not include JANA parameters, which come from
        /// JParameterManager::PrintParameters() instead.

        std::cout << std::endl;
        std::cout << "Usage:" << std::endl;
        std::cout << "    eicrecon [options] source1 source2 ..." << std::endl;
        std::cout << std::endl;

        std::cout << "Description:" << std::endl;
        std::cout << "    Command-line interface for running JANA plugins. This can be used to" << std::endl;
        std::cout << "    read in events and process them. Command-line flags control configuration" << std::endl;
        std::cout << "    while additional arguments denote input files, which are to be loaded and" << std::endl;
        std::cout << "    processed by the appropriate EventSource plugin." << std::endl;
        std::cout << std::endl;

        PrintUsageOptions();
        PrintUsageExample();
    }

    void PrintVersion() {
        std::cout << "EICrecon version: " << EICRECON_APP_VERSION_STR << std::endl;
    }

    void PrintJANAVersion() {
        std::cout << "JANA version: " << JVersion::GetVersion() << std::endl;
    }

    void PrintDefaultPlugins(std::vector<std::string> const& default_plugins) {
        std::cout << "\n List default plugins:\n\n";
        printPluginNames(default_plugins);
        std::cout << std::endl << std::endl;
    }

    void GetPluginNamesInDir(std::set<std::string> & plugin_names, std::string dir_str) {
        // Edge case handler: taking care of invalid and empty dirs
        if (std::filesystem::is_directory(dir_str) == false)
            return;
        if (std::filesystem::is_empty(dir_str))
            return;

        std::string full_path, filename;
        for (const auto & entry : std::filesystem::directory_iterator(dir_str)) {
            full_path = std::string(entry.path());   // Example: "/usr/local/plugins/Tutorial.so"
            filename = full_path.substr(full_path.find_last_of("/") + 1);  // Example: "Tutorial.so"
            if (filename.substr(filename.size() - 3) == ".so") {
                std::string s = filename.substr(0, filename.size() - 3);
//                std::cout << filename << "==> "  << s << std::endl;
                plugin_names.insert(s);
            }
        }
    }

    /// Get the plugin names by searching for files named as *.so under $JANA_PLUGIN_PATH and $EICrecon_MY/plugins.
    /// @note It does not guarantee any effectiveness of the plugins.
    void GetPluginNamesFromEnvPath(std::set<std::string> & plugin_names, const char* env_var) {
        std::string dir_path, paths;

        const char* env_p = getenv(env_var);
        if (env_p) {
            if (strcmp(env_var, "EICrecon_MY") == 0) {
                paths = std::string(env_p) + "/plugins";
            }
            else {
                paths = std::string(env_p);
            }

            std::stringstream envvar_ss(paths);
            while (getline(envvar_ss, dir_path, ':')) {
                GetPluginNamesInDir(plugin_names, dir_path);
            }
        }
    }

    std::vector<std::string> GetAvailablePluginNames(std::vector<std::string> const& default_plugins) {
        // Use set to remove duplicates.
        /// @note The plugins will be override if you use the same plugin name at different paths.
        std::set<std::string> set_plugin_name;
        for (std::string s : default_plugins)
            set_plugin_name.insert(s);

        jana::GetPluginNamesFromEnvPath(set_plugin_name, "JANA_PLUGIN_PATH");
        jana::GetPluginNamesFromEnvPath(set_plugin_name, "EICrecon_MY");

        std::vector<std::string> plugin_names(set_plugin_name.begin(), set_plugin_name.end());
        return plugin_names;
    }

    void PrintAvailablePlugins(std::vector<std::string> const& default_plugins) {
        std::cout << "\n List available plugins:\n\n";
        printPluginNames(GetAvailablePluginNames(default_plugins));
        std::cout << std::endl << std::endl;
    }

    bool HasPrintOnlyCliOptions(UserOptions& options, std::vector<std::string> const& default_plugins) {
        if (options.flags[jana::ShowUsage]) {
            jana::PrintUsage();
            return true;
        }
        if (options.flags[jana::ShowVersion]) {
            jana::PrintVersion();
            return true;
        }
        if (options.flags[jana::ShowJANAVersion]) {
            jana::PrintJANAVersion();
            return true;
        }
        if (options.flags[jana::ShowDefaultPlugins]) {
            jana::PrintDefaultPlugins(default_plugins);
            return true;
        }
        if (options.flags[jana::ShowAvailablePlugins]) {
            jana::PrintAvailablePlugins(default_plugins);
            return true;
        }
        return false;
    }

    /// Detect whether the cli params @param options.params contain "-Pplugins_to_ignore=...<erase_str>...".
    /// If true, delete @param erase_str from the original cli string "-Pplugins_to_ignore".
    bool HasExcludeDefaultPluginsInCliParams(UserOptions& options, const std::string erase_str) {
        auto has_ignore_plugins = options.params.find("plugins_to_ignore");
        if (has_ignore_plugins == options.params.end())
            return false;

        // Has cli option "-Pplugins_to_ignore". Look for @param erase_str
        size_t pos = has_ignore_plugins->second.find(erase_str);
        if (pos == std::string::npos)  // does not contain @param erase_str
            return false;

        // Detect @param flag_str. Delete flag_str from the original cli option.
        std::string ignore_str;
        if (erase_str.length() + pos == has_ignore_plugins->second.length()) { // @param flag_str is at the end
            ignore_str = has_ignore_plugins->second.erase(pos, erase_str.length());
        } else { // erase "<flag_str>," from "-Pplugins_to_ignore".
            ignore_str = has_ignore_plugins->second.erase(pos, erase_str.length() + 1);
        }
        options.params["plugins_to_ignore"] = ignore_str;
        return true;
    }

    void AddAvailablePluginsToOptionParams(UserOptions& options, std::vector<std::string> const& default_plugins) {

        std::set<std::string> set_plugins;
        // Add the plugins at $EICrecon_MY/plugins.
//        jana::GetPluginNamesFromEnvPath(set_plugins, "EICrecon_MY");  // disabled as we do not want to automatically add these

        std::string plugins_str;  // the complete plugins list
        for (std::string s : set_plugins)
            plugins_str += s + ",";

        // Add the default plugins into the plugin set if there is no
        // "-Pplugins_to_ignore=default (exclude all default plugins)" option
        if (HasExcludeDefaultPluginsInCliParams(options, "default") == false)
            /// @note: The sequence of adding the default plugins matters.
            /// Have to keep the original sequence to not causing troubles.
            for (std::string s : default_plugins) {
                plugins_str += s + ",";
            }

        // Insert other plugins in cli option "-Pplugins=pl1,pl2,..."
        auto has_cli_plugin_params = options.params.find("plugins");
        if (has_cli_plugin_params != options.params.end()) {
            plugins_str += has_cli_plugin_params->second;
            options.params["plugins"] = plugins_str;
        } else {
            options.params["plugins"] = plugins_str.substr(0, plugins_str.size() - 1); // exclude last ","
        }
    }

    JApplication* CreateJApplication(UserOptions& options) {

        auto para_mgr = new JParameterManager(); // JApplication owns params_copy, does not own eventSources

        // Add the cli options based on the user inputs
        for (auto pair : options.params) {
            para_mgr->SetParameter(pair.first, pair.second);
        }

        // Shut down the [INFO] msg of adding plugins, printing cpu info
        if (options.flags[ListFactories]) {
            para_mgr->SetParameter(
                    "log:off",
                    "JPluginLoader,JArrowProcessingController,JArrow"
                    );
        }

        if (options.flags[LoadConfigs]) {
            // If the user specified an external config file, we should definitely use that
            try {
                para_mgr->ReadConfigFile(options.load_config_file);
            }
            catch (JException &e) {
                std::cout << "Problem loading config file '" << options.load_config_file << "'. Exiting." << std::endl
                          << std::endl;
                exit(-1);
            }
            std::cout << "Loaded config file '" << options.load_config_file << "'." << std::endl << std::endl;
        }

        // If the user hasn't specified a timeout (on cmd line or in config file), set the timeout to something reasonably high
        if (para_mgr->FindParameter("jana:timeout") == nullptr) {
            para_mgr->SetParameter("jana:timeout", 180); // seconds
            para_mgr->SetParameter("jana:warmup_timeout", 180); // seconds
        }

        auto app = new JApplication(para_mgr);

        const char* env_p = getenv("EICrecon_MY");
        if( env_p ){
            app->AddPluginPath( std::string(env_p) + "/plugins" );
        }

        for (auto event_src : options.eventSources) {
            app->Add(event_src);
        }
        return app;
    }

    void AddDefaultPluginsToJApplication(JApplication* app, std::vector<std::string> const& default_plugins) {
        for (std::string s : default_plugins)
            app->AddPlugin(s);
    }

    void PrintFactories(JApplication* app) {
        std::cout << std::endl << "List all the factories:" << std::endl << std::endl;
        printFactoryTable(app->GetComponentSummary());
        std::cout << std::endl;
    }

    void PrintPodioCollections(JApplication* app) {
        if (app->GetJParameterManager()->Exists("PODIO:PRINT_TYPE_TABLE")) {
            bool print_type_table = app->GetParameterValue<bool>("podio:print_type_table");

            // cli criteria: Ppodio:print_type_table=1
            if (print_type_table) {
                auto event_sources = app->GetService<JComponentManager>()->get_evt_srces();
                for (auto event_source : event_sources) {
//                    std::cout << event_source->GetPluginName() << std::endl;  // podio.so
//                    std::cout << event_source->GetResourceName() << std::endl;
                    if (event_source->GetPluginName().find("podio") != std::string::npos)
                        event_source->DoInitialize();
                }
            }

        }
    }

    void PrintConfigParameters(JApplication* app){
        /// Print a table of the currently defined configuration parameters.
        /// n.b. this mostly duplicates a call to app->GetJParameterManager()->PrintParameters()
        /// but avoids the issue it has of setting the values column to same
        /// width for all parameters. (That leads to lots of whitespace being
        /// printed due to the very long podio:output_include_collections param.

        // Determine column widths
        auto params = app->GetJParameterManager()->GetAllParameters();
        size_t max_key_length = 0;
        size_t max_val_length = 0;
        size_t max_max_val_length = 32; // maximum width allowed for column.
        for( auto &[key, p] : params ){
            if( key.length() > max_key_length ) max_key_length = key.length();
            if( p->GetValue().length() > max_val_length ){
                if( p->GetValue().length() <= max_max_val_length ) max_val_length = p->GetValue().length();
            }
        }

        std::cout << "\nConfiguration Parameters:" << std::endl;
        std::cout << "Name" + std::string(max_key_length-4, ' ') << " : ";
        std::cout << "Value" + std::string(max_val_length-5, ' ') << " : ";
        std::cout << "Description" << std::endl;
        std::cout << std::string(max_key_length+max_val_length+20, '-') << std::endl;
        for( auto &[key, p] : params ){
            std::stringstream ss;
            int key_length_diff = max_key_length - key.length();
            if( key_length_diff>0 ) ss << std::string(key_length_diff, ' ');
            ss << key;
            ss << " | ";

            int val_length_diff = max_val_length - p->GetValue().length();
            if( val_length_diff>0 ) ss << std::string(val_length_diff, ' ');
            ss << p->GetValue();
            ss << " | ";
            ss << p->GetDescription();

            std::cout << ss.str() << std::endl;
        }
        std::cout << std::string(max_key_length+max_val_length+20, '-') << std::endl;
    }

    int Execute(JApplication* app, UserOptions &options) {

        std::cout << std::endl;

        // std::cout << "JANA " << JVersion::GetVersion() << " [" << JVersion::GetRevision() << "]" << std::endl;

        if (options.flags[ShowConfigs]) {
            // Load all plugins, collect all parameters, exit without running anything
            app->Initialize();
            if (options.flags[Benchmark]) {
                JBenchmarker benchmarker(app);  // Show benchmarking configs only if benchmarking mode specified
            }
//            app->GetJParameterManager()->PrintParameters(true);
            PrintConfigParameters(app);
        }
        else if (options.flags[DumpConfigs]) {
            // Load all plugins, dump parameters to file, exit without running anything
            app->Initialize();
            std::cout << std::endl << "Writing configuration options to file: " << options.dump_config_file
                      << std::endl;
            app->GetJParameterManager()->WriteConfigFile(options.dump_config_file);
        }
        else if (options.flags[Benchmark]) {
            JSignalHandler::register_handlers(app);
            // Run JANA in benchmark mode
            JBenchmarker benchmarker(app); // Benchmarking params override default params
            benchmarker.RunUntilFinished(); // Benchmarker will control JApp Run/Stop
        }
        else if (options.flags[ListFactories]) {
            app->Initialize();
            PrintFactories(app);

            // TODO: more elegant processing here
            PrintPodioCollections(app);
        }
        else {
            // Run JANA in normal mode
            try {
                printJANAHeaderIMG();
                JSignalHandler::register_handlers(app);
                app->Run();
            }
            catch (JException& e) {
                std::cout << "----------------------------------------------------------" << std::endl;
                std::cout << e << std::endl;
            }
            catch (std::runtime_error& e) {
                std::cout << "----------------------------------------------------------" << std::endl;
                std::cout << "Exception: " << e.what() << std::endl;
            }
        }
        return (int) app->GetExitCode();
    }


    UserOptions GetCliOptions(int nargs, char *argv[], bool expect_extra) {

        UserOptions options;
        parser::Parser val_parser;

        std::map<std::string, Flag> tokenizer;
        tokenizer["-h"] = ShowUsage;
        tokenizer["--help"] = ShowUsage;
        tokenizer["-v"] = ShowVersion;
        tokenizer["--version"] = ShowVersion;
        tokenizer["-j"] = ShowJANAVersion;
        tokenizer["--janaversion"] = ShowJANAVersion;
        tokenizer["-c"] = ShowConfigs;
        tokenizer["--configs"] = ShowConfigs;
        tokenizer["-l"] = LoadConfigs;
        tokenizer["--loadconfigs"] = LoadConfigs;
        tokenizer["-d"] = DumpConfigs;
        tokenizer["--dumpconfigs"] = DumpConfigs;
        tokenizer["-b"] = Benchmark;
        tokenizer["--benchmark"] = Benchmark;
        tokenizer["-L"] = ListFactories;
        tokenizer["--list-factories"] = ListFactories;
        tokenizer["--list-default-plugins"] = ShowDefaultPlugins;
        tokenizer["--list-available-plugins"] = ShowAvailablePlugins;

        // `eicrecon` has the same effect with `eicrecon -h`
        if (nargs == 1) {
            options.flags[ShowUsage] = true;
        }

        for (int i = 1; i < nargs; i++) {

            std::string arg = argv[i];
            // std::cout << "Found arg " << arg << std::endl;

            if (argv[i][0] != '-') {
                options.eventSources.push_back(arg);
                continue;
            }

            switch (tokenizer[arg]) {

                case Benchmark:
                    options.flags[Benchmark] = true;
                    break;

                case ShowUsage:
                    options.flags[ShowUsage] = true;
                    break;

                case ShowVersion:
                    options.flags[ShowVersion] = true;
                    break;

                case ShowJANAVersion:
                    options.flags[ShowJANAVersion] = true;
                    break;

                case ShowConfigs:
                    options.flags[ShowConfigs] = true;
                    break;

                case LoadConfigs:
                    options.flags[LoadConfigs] = true;
                    if (i + 1 < nargs && argv[i + 1][0] != '-') {
                        options.load_config_file = argv[i + 1];
                        i += 1;
                    } else {
                        options.load_config_file = "jana.config";
                    }
                    break;

                case DumpConfigs:
                    options.flags[DumpConfigs] = true;
                    if (i + 1 < nargs && argv[i + 1][0] != '-') {
                        options.dump_config_file = argv[i + 1];
                        i += 1;
                    } else {
                        options.dump_config_file = "jana.config";
                    }
                    break;

                case ListFactories:
                    options.flags[ListFactories] = true;
                    break;

                case ShowDefaultPlugins:
                    options.flags[ShowDefaultPlugins] = true;
                    break;

                case ShowAvailablePlugins:
                    options.flags[ShowAvailablePlugins] = true;
                    break;

                // TODO: add exclude plugin options
                case Unknown:
                    if (argv[i][0] == '-' && argv[i][1] == 'P') {

                        size_t pos = arg.find("=");
                        if ((pos != std::string::npos) && (pos > 2)) {
                            std::string key = arg.substr(2, pos - 2);
                            std::string val = arg.substr(pos + 1);
                            if (options.params.find(key) != options.params.end()) {
                                std::cout << "Duplicate parameter '" << arg << "' ignored" << std::endl;
                            } else {
                                auto val_parsed = val_parser.dd4hep_to_string(val,key);
                                options.params.insert({ key, val_parsed.result });
                                if (!val_parsed.success)
                                    std::cerr << "ERROR: failed to parse '" << arg << "'" << std::endl;
                            }
                        } else {
                            std::cout << "Invalid JANA parameter '" << arg
                                      << "': Expected format -Pkey=value" << std::endl;
                            options.flags[ShowConfigs] = true;
                        }
                    } else {
                        if (!expect_extra) {
                            std::cout << "Invalid command line flag '" << arg << "'" << std::endl;
                            options.flags[ShowUsage] = true;
                        }
                    }
            }
        }
        return options;
    }
}
