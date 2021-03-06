// base/time/stopwatch.h - Class for measuring spans of time
// Copyright © 2016 by Donald King <chronos@chronos-tachyon.net>
// Available under the MIT License. See LICENSE for details.

#ifndef BASE_TIME_STOPWATCH_H
#define BASE_TIME_STOPWATCH_H

#include <utility>

#include "base/time/clock.h"
#include "base/time/duration.h"
#include "base/time/time.h"

namespace base {
namespace time {

// Stopwatch is a class for measuring Durations on a MonotonicClock.
//
// Example usage:
//
//    base::time::Stopwatch stopwatch;
//    stopwatch.start();
//    ...;
//    stopwatch.stop();
//    auto dur = stopwatch.elapsed();
//    LOG(INFO) << "Operation took " << dur.microseconds() << " µs";
//
class Stopwatch {
 public:
  // Measurement is a RAII helper class for automatically stopping a Stopwatch.
  //
  // Example usage:
  //
  //    base::time::Stopwatch stopwatch;
  //    if (expensive) {
  //      auto measurement = stopwatch.measure();
  //      ...;  // perform expensive operation
  //    }
  //    auto dur = stopwatch.elapsed();
  //    LOG(INFO) << "Operation took " << dur.microseconds() << " µs";
  //
  class Measurement {
   public:
    Measurement(const Measurement&) = delete;
    Measurement& operator=(const Measurement&) = delete;

    Measurement() noexcept : ptr_(nullptr) {}
    explicit Measurement(Stopwatch* ptr);
    Measurement(Measurement&& other) noexcept;
    Measurement& operator=(Measurement&& other) noexcept;
    ~Measurement() noexcept;

    void swap(Measurement& other) noexcept;
    bool valid() const noexcept { return !!ptr_; }
    void assert_valid() const;
    void start();
    void stop();
    void release();

   private:
    Stopwatch* ptr_;
  };

  // Stopwatches are constructed from MonotonicClocks.
  // The default constructor uses the system_monotonic_clock().
  Stopwatch(MonotonicClock c) noexcept : clock_(std::move(c)),
                                         running_(false) {}
  Stopwatch() : Stopwatch(system_monotonic_clock()) {}

  // Stopwatches are neither copyable nor moveable.
  Stopwatch(const Stopwatch&) = delete;
  Stopwatch(Stopwatch&&) = delete;
  Stopwatch& operator=(const Stopwatch&) = delete;
  Stopwatch& operator=(Stopwatch&&) = delete;

  // Checks the state of the Stopwatch.
  bool running() const noexcept { return running_; }
  void assert_stopped() const;
  void assert_running() const;

  // Starts a measurement.
  void start();

  // Concludes a measurement.
  void stop();

  // Resets all measurements.
  void reset();

  // Returns <elapsed time, accumulated time> as a pair.
  // - "Elapsed time" is the duration from start() to stop().
  // - "Accumulated time" is the sum of elapsed times since the last reset().
  std::pair<Duration, Duration> durations() const;
  Duration elapsed() const { return durations().first; }
  Duration cumulative() const { return durations().second; }

  // Starts the Stopwatch, and creates a Measurement subclass to stop it.
  Measurement measure() {
    start();
    return Measurement(this);
  }

 private:
  MonotonicClock clock_;
  MonotonicTime start_;
  MonotonicTime stop_;
  Duration cumulative_;
  bool running_;
};

inline Stopwatch::Measurement::Measurement(Measurement&& other) noexcept
    : ptr_(other.ptr_) {
  other.ptr_ = nullptr;
}

inline Stopwatch::Measurement& Stopwatch::Measurement::operator=(
    Stopwatch::Measurement&& other) noexcept {
  release();
  swap(other);
  return *this;
}

inline Stopwatch::Measurement::Measurement(Stopwatch* ptr) : ptr_(ptr) {
  assert_valid();
  ptr_->assert_running();
}

inline Stopwatch::Measurement::~Measurement() noexcept { release(); }

inline void Stopwatch::Measurement::swap(Measurement& other) noexcept {
  using std::swap;
  swap(ptr_, other.ptr_);
}

inline void Stopwatch::Measurement::start() {
  assert_valid();
  ptr_->start();
}

inline void Stopwatch::Measurement::stop() {
  assert_valid();
  ptr_->stop();
}

inline void Stopwatch::Measurement::release() {
  if (ptr_ && ptr_->running()) ptr_->stop();
  ptr_ = nullptr;
}

inline void swap(Stopwatch::Measurement& a,
                 Stopwatch::Measurement& b) noexcept {
  a.swap(b);
}

}  // namespace time
}  // namespace base

#endif  // BASE_TIME_STOPWATCH_H
