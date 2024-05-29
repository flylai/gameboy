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

#define GB_ASSERT(COND) assert((COND))
