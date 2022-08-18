#ifndef PluginsConfig_h
#define PluginsConfig_h
#include <string>
#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>

namespace po = boost::program_options;

#define DEFAULT_DENSITY 25
#define DEFAULT_DISTANCE 20
#define DEFAULT_DISTANCE_OUTLIER_FACTOR 2

typedef po::variables_map ConfigMap;


namespace config {
    class PluginsConfig {
    public:
        po::options_description getDescriptionTemplate(bool useDefault=true) {
            po::options_description desc("Allowed options");
            if (useDefault) {
                desc.add_options()
                    ("dc", po::value<float>()->default_value(DEFAULT_DENSITY), "set density factor level")
                    ("rhoc", po::value<float>()->default_value(DEFAULT_DISTANCE), "set distance factor level")
                    ("outlierDeltaFactor", po::value<float>()->default_value(DEFAULT_DISTANCE_OUTLIER_FACTOR), "set delta outlier distance factor")
                ;
            } else {
                desc.add_options()
                    ("dc", po::value<float>(), "set density factor level")
                    ("rhoc", po::value<float>(), "set distance factor level")
                    ("outlierDeltaFactor", po::value<float>(), "set delta outlier distance factor")
                ;
            }
            return desc;
        }

        PluginsConfig(const std::string& path) {
            auto desc = getDescriptionTemplate();

            std::ifstream configFile(path);
            po::store(po::parse_config_file(configFile, desc), configVariablesMap_);
            po::notify(configVariablesMap_);

            configFile.close();

            std::ofstream replacedConfigFile(path);
            replacedConfigFile << "dc=" << configVariablesMap_["dc"].as<float>() << std::endl
                                << "rhoc=" << configVariablesMap_["rhoc"].as<float>() << std::endl
                                << "outlierDeltaFactor=" << configVariablesMap_["outlierDeltaFactor"].as<float>();
            replacedConfigFile.close();
        }

        void updateParamsFromCLIArgs(int argc, char** argv) {
            auto desc = getDescriptionTemplate(false);
            po::variables_map vm;
            po::store(po::command_line_parser(argc, argv).options(desc).allow_unregistered().run(), vm);
            po::notify(vm);

            if (vm.count("dc")) {
                 configVariablesMap_.at("dc").value() = (boost::any)vm["dc"].as<float>();
            }
            if (vm.count("rhoc")) {
                 configVariablesMap_.at("rhoc").value() = (boost::any)vm["rhoc"].as<float>();
            }
            if (vm.count("outlierDeltaFactor")) {
                 configVariablesMap_.at("outlierDeltaFactor").value() = (boost::any)vm["outlierDeltaFactor"].as<float>();
            }
        }

        const ConfigMap& configMap() {
            return configVariablesMap_;
        }
    private:
        ConfigMap configVariablesMap_;
    };
}
#endif