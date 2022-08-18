#include <fstream>
#include <memory>
#include "Framework/ESProducer.h"
#include "Framework/EventSetup.h"
#include "Framework/ESPluginFactory.h"

class CLUEESProducer : public edm::ESProducer {
public:
  CLUEESProducer(std::filesystem::path const& inputFile) : data_{inputFile} {}
  void produce(edm::EventSetup& eventSetup);

private:
  std::filesystem::path data_;
};

void CLUEESProducer::produce(edm::EventSetup& eventSetup) {
  
}

DEFINE_FWK_EVENTSETUP_MODULE(CLUEESProducer);