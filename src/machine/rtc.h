#pragma once

#include <chrono>
#include <functional>
#include <thread>

#include "common/type.h"

namespace gb {

class RTC {
public:
  using TimerTask = std::function<u8()>;

  void addTask(const TimerTask &task) { tasks_.push_back(task); }

  void stop() { stop_ = true; }

  void run() {
    stop_ = false;
    while (!stop_) {
      auto now    = std::chrono::high_resolution_clock::now();
      auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
      auto diff   = now_ns - last_update_;
      //      if (diff >= SEQ) {
      for (const auto &task: tasks_) {
        // Only CPU will return T-cycle, other tasks will return 0.
        task();
      }
      last_update_ = now_ns;
      //      }
      //      std::this_thread::sleep_for(std::chrono::duration<u64, std::nano>(static_cast<u64>(SEQ)));
    }
  }

private:
  static constexpr u32 FREQUENCY = 4'194'304;
  static constexpr f64 SEQ       = (1.0 / FREQUENCY) * 1e9;

  std::vector<TimerTask> tasks_;
  i64 last_update_{};
  bool stop_{true};
};

} // namespace gb
