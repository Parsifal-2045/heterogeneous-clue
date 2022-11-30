#ifndef AlpakaCore_initialise_h
#define AlpakaCore_initialise_h

#include "AlpakaCore/alpakaConfig.h"

namespace cms::alpakatools {

  template <typename TPlatform>
  void initialise();

  // explicit template instantiation declaration
#ifdef ALPAKA_ACC_CPU_B_SEQ_T_SEQ_PRESENT
  extern template void initialise<alpaka_serial_sync::Platform>();
#endif
#ifdef ALPAKA_ACC_CPU_B_TBB_T_SEQ_PRESENT
  extern template void initialise<alpaka_tbb_async::Platform>();
#endif
#ifdef ALPAKA_ACC_GPU_CUDA_PRESENT
  extern template void initialise<alpaka_cuda_async::Platform>();
#endif
#ifdef ALPAKA_ACC_GPU_HIP_PRESENT
  extern template void initialise<alpaka_rocm_async::Platform>();
#endif
#ifdef ALPAKA_SYCL_ONEAPI_CPU
  extern template void initialise<alpaka_cpu_sycl::Platform>();
#endif
#ifdef ALPAKA_SYCL_ONEAPI_GPU
  extern template void initialise<alpaka_gpu_sycl::Platform>();
#endif

}  // namespace cms::alpakatools

#endif  // AlpakaCore_initialise_h
