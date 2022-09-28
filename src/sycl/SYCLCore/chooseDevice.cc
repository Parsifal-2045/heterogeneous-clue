#include <iostream>
#include <vector>

#include <CL/sycl.hpp>

#include "chooseDevice.h"

namespace cms::sycltools {
  std::vector<sycl::device> const& discoverDevices() {
    static std::vector<sycl::device> temp;
    std::vector<sycl::device> cpus = sycl::device::get_devices(sycl::info::device_type::cpu);
    std::vector<sycl::device> gpus = sycl::device::get_devices(sycl::info::device_type::gpu);
    std::vector<sycl::device> hosts = sycl::device::get_devices(sycl::info::device_type::host);
    for (auto it = cpus.begin(); it != cpus.end(); it++) {
      if (it + 1 == cpus.end()) {
        break;
      }
      if ((*it).get_info<sycl::info::device::name>() == (*(it + 1)).get_info<sycl::info::device::name>() and
          (*it).get_backend() == (*(it + 1)).get_backend() and
          (*(it + 1)).get_info<sycl::info::device::driver_version>() <
              (*it).get_info<sycl::info::device::driver_version>()) {
        cpus.erase(it + 1);
      }
    }
    temp.insert(temp.end(), cpus.begin(), cpus.end());

    for (auto it = gpus.begin(); it != gpus.end(); it++) {
      if (it + 1 == gpus.end()) {
        break;
      }
      if ((*it).get_info<sycl::info::device::name>() == (*(it + 1)).get_info<sycl::info::device::name>() and
          (*it).get_backend() == (*(it + 1)).get_backend() and
          (*(it + 1)).get_info<sycl::info::device::driver_version>() <
              (*it).get_info<sycl::info::device::driver_version>()) {
        gpus.erase(it + 1);
      }
    }

#ifdef USE_SUB_DEVICES
    for (auto gpu : gpus) {
      if (gpu.get_backend() == sycl::backend::ext_oneapi_level_zero) {
        try {
          std::vector<sycl::device> subDevices =
              gpu.create_sub_devices<sycl::info::partition_property::partition_by_affinity_domain>(
                  sycl::info::partition_affinity_domain::next_partitionable);
          temp.insert(temp.end(), subDevices.begin(), subDevices.end());
        } catch (sycl::exception const& e) {
          std::cerr << "GPU " << gpu.get_info<sycl::info::device::name>()
                    << " does not support splitting into multiple sub devices. Falling back to a single device"
                    << std::endl;
          temp.emplace_back(gpu);
        }
      } else {
        temp.emplace_back(gpu);
      }
    }
#else
    temp.insert(temp.end(), gpus.begin(), gpus.end());
#endif
    temp.insert(temp.end(), hosts.begin(), hosts.end());
    return temp;
  }

  std::vector<sycl::device> const& enumerateDevices(bool verbose) {
    static const std::vector<sycl::device> devices = discoverDevices();

    if (verbose) {
      std::cerr << "Found " << devices.size() << " SYCL devices:" << std::endl;
      for (auto const& device : devices)
        std::cerr << "  - " << device.get_backend() << ' ' << device.get_info<cl::sycl::info::device::name>() << " ["
                  << device.get_info<sycl::info::device::driver_version>() << "]" << std::endl;
      std::cerr << std::endl;
    }
    return devices;
  }

  sycl::device chooseDevice(edm::StreamID id) {
    auto const& devices = enumerateDevices();
    auto const& device = devices[id % devices.size()];
    std::cerr << "EDM stream " << id << " offload to " << device.get_info<cl::sycl::info::device::name>()
              << " on backend " << device.get_backend() << std::endl;
    return device;
  }
}  // namespace cms::sycltools
