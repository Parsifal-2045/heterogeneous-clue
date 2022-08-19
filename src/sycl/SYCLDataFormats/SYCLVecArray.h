#ifndef SYCL_VEC_ARRAY_H
#define SYCL_VEC_ARRAY_H

#include <CL/sycl.hpp>
#include "SYCLCore/syclAtomic.h"

//
// Author: Felice Pantaleo, CERN
//
namespace cms {
  namespace sycltools {

    template <class T, int maxSize>
    struct VecArray {
      inline constexpr int push_back_unsafe(const T &element) {
        auto previousSize = m_size;
        m_size++;
        if (previousSize < maxSize) {
          m_data[previousSize] = element;
          return previousSize;
        } else {
          --m_size;
          return -1;
        }
      }

      template <class... Ts>
      constexpr int emplace_back_unsafe(Ts &&...args) {
        auto previousSize = m_size;
        m_size++;
        if (previousSize < maxSize) {
          (new (&m_data[previousSize]) T(std::forward<Ts>(args)...));
          return previousSize;
        } else {
          --m_size;
          return -1;
        }
      }

      inline constexpr T &back() const {
        if (m_size > 0) {
          return m_data[m_size - 1];
        } else
          return T();  // undefined behaviour
      }

      // thread-safe version of the vector, when used in a CUDA kernel
      int push_back(const T &element) {
        auto previousSize = AtomicAdd(&m_size, 1);
        if (previousSize < maxSize) {
          m_data[previousSize] = element;
          return previousSize;
        } else {
          AtomicSub(&m_size, 1);
          return -1;
        }
      }

      template <class... Ts>
      int emplace_back(Ts &&...args) {
        auto previousSize = AtomicAdd(&m_size, 1);
        if (previousSize < maxSize) {
          (new (&m_data[previousSize]) T(std::forward<Ts>(args)...));
          return previousSize;
        } else {
          AtomicSub(&m_size, 1);
          return -1;
        }
      }

      inline T pop_back() {
        if (m_size > 0) {
          auto previousSize = m_size--;
          return m_data[previousSize - 1];
        } else
          return T();
      }

      inline constexpr T const *begin() const { return m_data; }
      inline constexpr T const *end() const { return m_data + m_size; }
      inline constexpr T *begin() { return m_data; }
      inline constexpr T *end() { return m_data + m_size; }
      inline constexpr int size() const { return m_size; }
      inline constexpr T &operator[](int i) { return m_data[i]; }
      inline constexpr const T &operator[](int i) const { return m_data[i]; }
      inline constexpr void reset() { m_size = 0; }
      inline constexpr int capacity() const { return maxSize; }
      inline constexpr T const *data() const { return m_data; }
      inline constexpr void resize(int size) { m_size = size; }
      inline constexpr bool empty() const { return 0 == m_size; }
      inline constexpr bool full() const { return maxSize == m_size; }

      int m_size = 0;

      T m_data[maxSize];
    };

  }  // namespace sycltools
}  // namespace cms

#endif  // SYCL_VEC_ARRAY_H
