#include "effects/Ticker.hpp"

namespace itsme::effects {

Ticker::Ticker(std::chrono::milliseconds interval, std::function<void()> onTick)
    : onTick_(std::move(onTick)), interval_(interval), thread_([this] { loop(); }) {}

Ticker::~Ticker() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    stop_ = true;
  }
  cv_.notify_all();
  if (thread_.joinable()) thread_.join();
}

void Ticker::setInterval(std::chrono::milliseconds interval) {
  std::lock_guard<std::mutex> lock(mutex_);
  interval_ = interval;
}

void Ticker::loop() {
  std::unique_lock<std::mutex> lock(mutex_);
  while (!stop_) {
    const auto wait = interval_;
    if (cv_.wait_for(lock, wait, [this] { return stop_; })) break;
    lock.unlock();
    onTick_();
    lock.lock();
  }
}

}  // namespace itsme::effects
