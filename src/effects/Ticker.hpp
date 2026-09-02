#pragma once
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

namespace itsme::effects {

class Ticker {
 public:
  Ticker(std::chrono::milliseconds interval, std::function<void()> onTick);
  ~Ticker();
  Ticker(const Ticker&) = delete;
  Ticker& operator=(const Ticker&) = delete;
  void setInterval(std::chrono::milliseconds interval);

 private:
  void loop();
  std::function<void()> onTick_;
  std::chrono::milliseconds interval_;
  bool stop_ = false;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::thread thread_;
};

}  // namespace itsme::effects
