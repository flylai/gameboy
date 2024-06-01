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
  return val & ~(1 << (n));
}

template<typename T>
inline static T getBitN(T val, int n) {
  return val >> n & 1;
}

#define GB_ASSERT(COND) assert((COND))
