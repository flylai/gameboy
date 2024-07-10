#pragma once

#include <condition_variable>

#include "machine/gameboy.h"
#include "machine/serial/serial_buffer.h"

namespace gb {

class Monitor {
public:
  enum CheckStatus {
    kFAILED = 0,
    kSUCCESS,
    kUNKNOWN,
  };

  using CheckFunction = std::function<CheckStatus(const std::vector<u8> &)>;

  Monitor(GameBoy *gb, const CheckFunction &chk_func) : gb_(gb), check_function(chk_func) {
    GB_ASSERT(gb_ != nullptr);
    GB_ASSERT(check_function != nullptr);
  }

  bool run(u64 timeout) {
    thread_ = std::thread([&]() {
      std::unique_lock<std::mutex> lk(cv_mutex_);
      cv_.wait_for(lk, std::chrono::seconds(timeout));
      gb_->rtc_.stop();
    });

    serial_buffer_.serialObserver([&](const auto &v) { check(v); });
    gb_->serial_.buffer(&serial_buffer_);
    thread_.detach();
    gb_->rtc_.run();
    return success_;
  }

private:
  void check(const std::vector<u8> &v) {
    CheckStatus status = check_function(v);
    if (status != kUNKNOWN) {
      success_ = !!status;
      cv_.notify_all();
    }
  }

private:
  GameBoy *gb_{};
  CheckFunction check_function{};
  std::thread thread_;
  SerialBuffer serial_buffer_;
  std::vector<u8> expect_;
  std::condition_variable cv_;
  std::mutex cv_mutex_;
  bool success_{};
};

} // namespace gb
