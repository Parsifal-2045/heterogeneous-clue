#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

#include <tbb/global_control.h>
#include <tbb/info.h>
#include <tbb/task_arena.h>

#include <CL/sycl.hpp>

#include "SYCLCore/chooseDevice.h"
#include "EventProcessor.h"
#include "PosixClockGettime.h"

namespace {
  void print_help(std::string const& name) {
    std::cout
        << name
        << ": [--numberOfThreads NT] [--numberOfStreams NS] [--maxEvents ME] [--data PATH] [--transfer] [--validation] "
           "[--empty]\n\n"
        << "Options\n"
        << "--device             Specifies the device which should run the code (default not set. Possibilities are: "
           "cpu, gpu or acc\n"
        << " --numberOfThreads   Number of threads to use (default 1, use 0 to use all CPU cores)\n"
        << " --numberOfStreams   Number of concurrent events (default 0 = numberOfThreads)\n"
        << " --maxEvents         Number of events to process (default -1 for all events in the input file)\n"
        << " --runForMinutes     Continue processing the set of 1000 events until this many minutes have passed "
           "(default -1 for disabled; conflicts with --maxEvents)\n"
        << " --data              Path to the 'data' directory (default 'data' in the directory of the executable)\n"
        << " --transfer          Transfer results from GPU to CPU (default is to leave them on GPU)\n"
        << " --validation        Run (rudimentary) validation at the end (implies --transfer)\n"
        << " --empty             Ignore all producers (for testing only)\n"
        << std::endl;
  }
}  // namespace

int main(int argc, char** argv) try {
  // Parse command line arguments
  setenv("SYCL_DEVICE_FILTER", "cpu,gpu", true);
  std::vector<std::string> args(argv, argv + argc);
  int numberOfThreads = 1;
  int numberOfStreams = 0;
  int maxEvents = -1;
  int runForMinutes = -1;
  std::filesystem::path datadir;
  bool transfer = false;
  bool validation = false;
  bool empty = false;
  for (auto i = args.begin() + 1, e = args.end(); i != e; ++i) {
    if (*i == "-h" or *i == "--help") {
      print_help(args.front());
      return EXIT_SUCCESS;
    } else if (*i == "--device") {
      ++i;
      std::string device = *i;
      setenv("SYCL_DEVICE_FILTER", device.c_str(), true);
    } else if (*i == "--numberOfThreads") {
      ++i;
      numberOfThreads = std::stoi(*i);
    } else if (*i == "--numberOfStreams") {
      ++i;
      numberOfStreams = std::stoi(*i);
    } else if (*i == "--maxEvents") {
      ++i;
      maxEvents = std::stoi(*i);
    } else if (*i == "--runForMinutes") {
      ++i;
      runForMinutes = std::stoi(*i);
    } else if (*i == "--data") {
      ++i;
      datadir = *i;
    } else if (*i == "--transfer") {
      transfer = true;
    } else if (*i == "--validation") {
      transfer = true;
      validation = true;
    } else if (*i == "--empty") {
      empty = true;
    } else {
      std::cout << "Invalid parameter " << *i << std::endl << std::endl;
      print_help(args.front());
      return EXIT_FAILURE;
    }
  }
  if (maxEvents >= 0 and runForMinutes >= 0) {
    std::cout << "Got both --maxEvents and --runForMinutes, please give only one of them" << std::endl;
    return EXIT_FAILURE;
  }
  if (numberOfThreads == 0) {
    numberOfThreads = tbb::info::default_concurrency();
  }
  if (numberOfStreams == 0) {
    numberOfStreams = numberOfThreads;
  }
  if (datadir.empty()) {
    datadir = std::filesystem::path(args[0]).parent_path() / "data";
  }
  if (not std::filesystem::exists(datadir)) {
    std::cout << "Data directory '" << datadir << "' does not exist" << std::endl;
    return EXIT_FAILURE;
  }

  // Initialise the SYCL runtime
  cms::sycltools::enumerateDevices(true);

  // Initialise the EventProcessor
  std::vector<std::string> edmodules;
  std::vector<std::string> esmodules;
  if (not empty) {
    edmodules = {"PointsCloudProducer", "CLUESYCLClusterizer"};
    esmodules = {"PointsCloudESProducer"};
    if (transfer) {
      esmodules.emplace_back("CLUEOutputESProducer");
      edmodules.emplace_back("CLUEOutputProducer");
    }
    if (validation) {
      esmodules.emplace_back("ValidatorPointsCloudESProducer");
      edmodules.emplace_back("ValidatorPointsCloudToSYCL");
      esmodules.emplace_back("CLUEValidatorESProducer");
      edmodules.emplace_back("CLUEValidator");
    }
  }
  edm::EventProcessor processor(
      maxEvents, runForMinutes, numberOfStreams, std::move(edmodules), std::move(esmodules), datadir, validation);
  if (runForMinutes < 0) {
    std::cout << "Processing " << processor.maxEvents() << " events, of which " << numberOfStreams
              << " concurrently, with " << numberOfThreads << " threads." << std::endl;
  } else {
    std::cout << "Processing for about " << runForMinutes << " minutes with " << numberOfStreams
              << " concurrent events and " << numberOfThreads << " threads." << std::endl;
  }

  // Initialize he TBB thread pool
  tbb::global_control tbb_max_threads{tbb::global_control::max_allowed_parallelism,
                                      static_cast<std::size_t>(numberOfThreads)};

  // Run work
  auto cpu_start = PosixClockGettime<CLOCK_PROCESS_CPUTIME_ID>::now();
  auto start = std::chrono::high_resolution_clock::now();
  try {
    tbb::task_arena arena(numberOfThreads);
    arena.execute([&] { processor.runToCompletion(); });
  } catch (std::runtime_error& e) {
    std::cout << "\n----------\nCaught std::runtime_error" << std::endl;
    std::cout << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (std::exception& e) {
    std::cout << "\n----------\nCaught std::exception" << std::endl;
    std::cout << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cout << "\n----------\nCaught exception of unknown type" << std::endl;
    return EXIT_FAILURE;
  }
  auto cpu_stop = PosixClockGettime<CLOCK_PROCESS_CPUTIME_ID>::now();
  auto stop = std::chrono::high_resolution_clock::now();

  // Run endJob
  try {
    processor.endJob();
  } catch (std::runtime_error& e) {
    std::cout << "\n----------\nCaught std::runtime_error" << std::endl;
    std::cout << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (std::exception& e) {
    std::cout << "\n----------\nCaught std::exception" << std::endl;
    std::cout << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cout << "\n----------\nCaught exception of unknown type" << std::endl;
    return EXIT_FAILURE;
  }

  // Work done, report timing
  auto diff = stop - start;
  auto time = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(diff).count()) / 1e6;
  auto cpu_diff = cpu_stop - cpu_start;
  auto cpu = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(cpu_diff).count()) / 1e6;
  maxEvents = processor.processedEvents();
  std::cout << "Processed " << maxEvents << " events in " << std::scientific << time << " seconds, throughput "
            << std::defaultfloat << (maxEvents / time) << " events/s, CPU usage per thread: " << std::fixed
            << std::setprecision(1) << (cpu / time / numberOfThreads * 100) << "%" << std::endl;
  unsetenv("SYCL_DEVICE_FILTER");
  return EXIT_SUCCESS;
} catch (sycl::exception const& exc) {
  std::cerr << exc.what() << "Exception caught at file:" << __FILE__ << ", line:" << __LINE__ << std::endl;
  std::exit(1);
}
