#ifndef EventProcessor_h
#define EventProcessor_h

#include <filesystem>
#include <string>
#include <vector>

#include "Framework/EventSetup.h"
#include "Framework/PluginsConfig.h"

#include "PluginManager.h"
#include "StreamSchedule.h"
#include "Source.h"

namespace edm {
  class EventProcessor {
  public:
    explicit EventProcessor(int maxEvents,
                            int runForMinutes,
                            int numberOfStreams,
                            std::vector<std::string> const& path,
                            std::vector<std::string> const& esproducers,
                            std::filesystem::path const& datadir,
                            bool validation);

    explicit EventProcessor(int maxEvents,
                            int runForMinutes,
                            int numberOfStreams,
                            std::vector<std::string> const& path,
                            std::vector<std::string> const& esproducers,
                            std::filesystem::path const& datadir,
                            std::filesystem::path const& inputFile,
                            bool validation,
                            ConfigMap const& configMap);

    int maxEvents() const { return source_.maxEvents(); }
    int processedEvents() const { return source_.processedEvents(); }

    void runToCompletion();

    void endJob();

  private:
    edmplugin::PluginManager pluginManager_;
    ProductRegistry registry_;
    Source source_;
    EventSetup eventSetup_;
    std::vector<StreamSchedule> schedules_;
  };
}  // namespace edm

#endif
