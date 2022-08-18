#include <fstream>
#include <iostream>
#include <memory>
#include "Framework/ESProducer.h"
#include "Framework/EventSetup.h"
#include "Framework/ESPluginFactory.h"
#include "Framework/PluginsConfig.h"
#include "CLUEParams.h"

class CLUEESProducer : public edm::ConfigurableESProducer {
public:
  CLUEESProducer(std::filesystem::path const& inputFile, ConfigMap const& configMap)
    : data_{inputFile}, configMap_{configMap} {}
  void produce(edm::EventSetup& eventSetup);

private:
  std::filesystem::path data_;
  ConfigMap configMap_;
};

void CLUEESProducer::produce(edm::EventSetup& eventSetup) {
  std::unique_ptr<CLUEParams> params(new CLUEParams());
  params->dc = configMap_["dc"].as<float>();
  params->rhoc = configMap_["rhoc"].as<float>();
  params->outlierDeltaFactor = configMap_["outlierDeltaFactor"].as<float>();

  eventSetup.put(std::move(params));
}

DEFINE_FWK_EVENTSETUP_MODULE(CLUEESProducer);