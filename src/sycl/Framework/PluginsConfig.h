#ifndef PluginsConfig_h
#define PluginsConfig_h
#include <string>
#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>
#include <unordered_map>

namespace po = boost::program_options;

#define DEFAULT_DENSITY 25
#define DEFAULT_DISTANCE 20
#define DEFAULT_DISTANCE_OUTLIER_FACTOR 2

typedef std::unordered_map<std::string, std::string> KeyValueMap;

namespace config {
    class PluginsConfig {
    public:
        PluginsConfig(const std::string& path) {
            po::options_description desc("Allowed options");
            desc.add_options()
                ("rho", po::value<float>()->default_value(DEFAULT_DENSITY), "set density factor level")
                ("delta", po::value<float>()->default_value(DEFAULT_DISTANCE), "set distance factor level")
                ("dof", po::value<float>()->default_value(DEFAULT_DISTANCE_OUTLIER_FACTOR), "set delta outlier distance factor")
            ;

            po::variables_map vm;
            std::ifstream configFile(path);
            po::store(po::parse_config_file(configFile, desc), vm);
            po::notify(vm);

            configFile.close();

            std::ofstream replacedConfigFile(path);
            replacedConfigFile << "rho=" << vm["rho"].as<float>() << std::endl
                                << "delta=" << vm["delta"].as<float>() << std::endl
                                << "dof=" << vm["dof"].as<float>();
            replacedConfigFile.close();

            keyValuesMap_["rho"] = vm["rho"];
            keyValuesMap_["delta"] = vm["delta"];
            keyValuesMap_["dof"] = vm["dof"];
  std::cout << "Hola" << std::endl;


            std::cout << "Loaded config file " << path << std::endl;
        }

        void updateParamsFromCLIArgs() {}

        const KeyValueMap& keyValuesMap() {
            return keyValuesMap_;
        }

    private:
        KeyValueMap keyValuesMap_;
    };
}
#endif