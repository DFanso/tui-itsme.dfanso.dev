#pragma once
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>

namespace itsme::core {

// Runs `work` on a detached thread. The result is readable once ready() is true.
// `onDone` runs on the worker thread after the value is stored (use it to poke the UI loop).
template <class T>
class AsyncValue {
 public:
  static std::shared_ptr<AsyncValue> start(std::function<T()> work, std::function<void()> onDone) {
    auto self = std::make_shared<AsyncValue>();
    std::thread([self, work = std::move(work), onDone = std::move(onDone)] {
      T result = work();
      {
        std::lock_guard<std::mutex> lock(self->mutex_);
        self->value_ = std::move(result);
      }
      self->ready_.store(true, std::memory_order_release);
      if (onDone) onDone();
    }).detach();
    return self;
  }

  bool ready() const { return ready_.load(std::memory_order_acquire); }
  const T& get() const { return *value_; }  // only valid when ready()

 private:
  std::atomic<bool> ready_{false};
  std::mutex mutex_;
  std::optional<T> value_;
};

}  // namespace itsme::core
