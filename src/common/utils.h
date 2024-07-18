#pragma once

#include <cassert>

template<typename R, typename... U>
static inline bool inAnd(R cond, U... args) {
  return ((cond == args) && ...);
}

template<typename R, typename... U>
static inline bool inOr(R cond, U... args) {
  return ((cond == args) || ...);
}

template<typename T>
inline static T clearBitN(T val, int n) {
  return val & ~(1 << n);
}

template<typename T>
inline static T getBitN(T val, int n) {
  return val >> n & 1;
}

template<typename T>
inline static T setBitN(T val, int n) {
  return clearBitN(val, n) | (1 << n);
}

#define GB_ASSERT(COND) assert((COND))
#define GB_STATIC_ASSERT(COND, MSG) static_assert((COND), MSG)
#define INLINE __attribute__((always_inline)) inline