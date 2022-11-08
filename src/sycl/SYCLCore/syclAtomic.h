#ifndef HeterogeneousCore_SYCLUtilities_interface_syclAtomic_h
#define HeterogeneousCore_SYCLUtilities_interface_syclAtomic_h

#include <CL/sycl.hpp>
#include <cstdint>

namespace cms {
  namespace sycltools {

    template <typename T,  
              sycl::access::address_space addrSpace = cl::sycl::access::address_space::global_space,
              sycl::memory_scope Scope = cl::sycl::memory_scope::device,
              sycl::memory_order memOrder = cl::sycl::memory_order::relaxed>
    inline T atomic_fetch_add(T* addr, T operand){
    
      auto atm = sycl::atomic_ref<T, memOrder, Scope, addrSpace>(addr[0]);
      
      return atm.fetch_add(operand);
    }

    template <typename T,  
              sycl::access::address_space addrSpace = cl::sycl::access::address_space::global_space,
              sycl::memory_scope Scope = cl::sycl::memory_scope::device,
              sycl::memory_order memOrder = cl::sycl::memory_order::relaxed>
    inline T atomic_fetch_sub(T* addr, T operand) {
    
      auto atm = sycl::atomic_ref<T, memOrder, Scope, addrSpace>(addr[0]);
          
      return atm.fetch_sub(operand);
    }

    template <typename T,  
              sycl::access::address_space addrSpace = cl::sycl::access::address_space::global_space,
              sycl::memory_scope Scope = cl::sycl::memory_scope::device,
              sycl::memory_order memOrder = cl::sycl::memory_order::relaxed>
    inline T atomic_fetch_compare_inc(T *addr,T operand) {
    
      auto atm = sycl::atomic_ref<T, memOrder, Scope, addrSpace>(addr[0]);
      
      T old;
      while (true) {
        old = atm.load();
        if (old >= operand) {
          if (atm.compare_exchange_strong(old, 0))
            break;
        } else if (atm.compare_exchange_strong(old, old + 1))
          break;
      }
      return old;
    }

    template <typename T,  
              sycl::access::address_space addrSpace = cl::sycl::access::address_space::global_space,
              sycl::memory_scope Scope = cl::sycl::memory_scope::device,
              sycl::memory_order memOrder = cl::sycl::memory_order::relaxed>
    inline T atomic_fetch_min(T *addr, T operand) {
    
      auto atm = sycl::atomic_ref<T, memOrder, Scope, addrSpace>(addr[0]);  
                
      return atm.fetch_min(operand);
    }

    template <typename T,  
              sycl::access::address_space addrSpace = cl::sycl::access::address_space::global_space,
              sycl::memory_scope Scope = cl::sycl::memory_scope::device,
              sycl::memory_order memOrder = cl::sycl::memory_order::relaxed>
    inline T atomic_fetch_max(T *addr, T operand) {
    
      auto atm = sycl::atomic_ref<T, memOrder, Scope, addrSpace>(addr[0]); 
                 
    return atm.fetch_max(operand);
    }
    
    template <typename T,  
              sycl::access::address_space addrSpace = cl::sycl::access::address_space::global_space,
              sycl::memory_scope Scope = cl::sycl::memory_scope::device,
              sycl::memory_order memOrder = cl::sycl::memory_order::relaxed>
    inline T atomic_compare_exchange_strong(T *addr, T expected, T desired) {
                auto atm = cl::sycl::atomic_ref<T, memOrder, Scope, addrSpace>(addr[0]);
                atm.compare_exchange_strong(expected, desired, memOrder, memOrder);
        return expected;
      }
  }  // namespace sycltools
}  // namespace cms

#endif  // HeterogeneousCore_SYCLUtilities_interface_syclAtomic_h