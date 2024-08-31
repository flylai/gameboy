#pragma once

#include <chrono>
#include <condition_variable>
#include <functional>
#include <thread>

#include "common/type.h"

namespace gb {

class RTC {
public:
  using TimerTask = std::function<u8()>;

  void addTask(const TimerTask &task) { tasks_.push_back(task); }

  void stop() {
    {
      std::lock_guard<std::mutex> lock(pause_mutex_);
      stop_  = true;
      pause_ = true;
    }
    pause_cv_.notify_one();

    if (thread_.joinable()) {
      thread_.join();
    }
  }

  bool stopped() const { return stop_; }

  void pause() {
    std::lock_guard<std::mutex> lock(pause_mutex_);
    pause_ = true;
  }

  bool paused() const { return pause_; }

  void resume() {
    {
      std::lock_guard<std::mutex> lock(pause_mutex_);
      pause_ = false;
    }
    pause_cv_.notify_one();
  }

  INLINE static u64 getNS() {
    auto now      = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
  }

  void cpuSpeedLock(bool v) { unlock_cpu_speed_ = v; }

  u64 cpuSpeed() const { return cpu_speed_; }

  void run() {
    stop_ = false;
    while (!stop_) {
      // init time_accumulate_ to current time
      time_accumulate_ = getNS();

      {
        std::unique_lock<std::mutex> lock(pause_mutex_);
        pause_cv_.wait(lock, [&] { return !pause_ || stop_; });

        if (stop_) {
          break;
        }
      }

      while (!pause_) {
        auto now = getNS();

        if (unlock_cpu_speed_) {
          for (const auto &task: tasks_) {
            calculateCPUSpeed(task(), now);
          }
        } else {
          if (now >= time_accumulate_) {
            for (const auto &task: tasks_) {
              // Only CPU will return T-cycle, other tasks will return 0.
              u8 t_cycle = task();
              calculateCPUSpeed(t_cycle, now);
              time_accumulate_ += t_cycle * (u64) SEQ;
            }
          } else {
            // sleep until next cycle
            // it depends on the CPU speed, if the host CPU is too low
            // it may cause the emulator to run slower than the real speed.
            // to fixed, just decrease the `sleep` value.
            std::this_thread::sleep_for(std::chrono::nanoseconds((u64) SEQ));
          }
        }
      }
    }
  }

  void runWithNewThread() {
    if (thread_.joinable()) {
      thread_.join();
    }
    thread_ = std::thread(&RTC::run, this);
  }

  void calculateCPUSpeed(u64 cycle, u64 now) {
    t_cycle_count_ += cycle;
    auto duration = now - last_update_;

    if (duration >= 1e9) { // 1 second in nanoseconds
      cpu_speed_     = t_cycle_count_;
      t_cycle_count_ = 0;
      last_update_   = now;
    }
  }

private:
  static constexpr u32 FREQUENCY = 4'194'304;
  static constexpr f64 SEQ       = (1.0 / FREQUENCY) * 1e9;
  u64 time_accumulate_{};
  std::atomic<bool> unlock_cpu_speed_{};

  u64 cpu_speed_{};
  u64 t_cycle_count_{};
  u64 last_update_{};


  std::thread thread_;

  std::vector<TimerTask> tasks_;

  bool stop_{true};
  std::atomic<bool> pause_{};
  std::condition_variable pause_cv_;
  std::mutex pause_mutex_;
};

} // namespace gb
