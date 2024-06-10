#pragma once

#include <chrono>
#include <functional>
#include <thread>

#include "common/type.h"

namespace gb {

class RTC {
public:
  using TimerTask = std::function<void(u64)>;

  u32 addTask(const TimerTask &task) {
    tasks_[task_idx_] = task;
    return task_idx_++;
  }

  void removeTask(u32 task_idx) { tasks_.erase(task_idx); }

  void stop() { stop_ = true; }

  void run() {
    stop_ = false;
    while (!stop_) {
      auto now    = std::chrono::high_resolution_clock::now();
      auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
      auto diff   = now_ns - last_update_;
      if (diff >= SEQ) {
        for (const auto &[idx, task]: tasks_) {
          task(cycle_);
        }
        last_update_ = now_ns;
        cycle_++;
      }
      std::this_thread::sleep_for(std::chrono::duration<u64, std::nano>(static_cast<u64>(SEQ)));
    }
  }

private:
  static constexpr u32 FREQUENCY = 4'194'304;
  static constexpr f64 SEQ       = (1.0 / FREQUENCY) * 1e9;

  u32 task_idx_{};
  std::unordered_map<u32, TimerTask> tasks_;
  i64 last_update_{};
  u16 cycle_{};
  bool stop_{true};
};

} // namespace gb
