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
inline static constexpr T clearBitN(T val, int n) {
  return val & ~(1 << n);
}

template<typename T>
inline static T constexpr getBitN(T val, int n) {
  return val >> n & 1;
}

template<typename T>
inline static T setBitN(T val, int n) {
  return clearBitN(val, n) | (1 << n);
}

#ifdef NDEBUG
#define GB_ASSERT(IGNORE)
#else
#define GB_ASSERT(COND) assert((COND))
#endif

#define GB_STATIC_ASSERT(COND, MSG) static_assert((COND), MSG)
#define INLINE __attribute__((always_inline)) inline
#define GB_API __attribute__((visibility("default")))
