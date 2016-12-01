// Copyright © 2016 by Donald King <chronos@chronos-tachyon.net>
// Available under the MIT License. See LICENSE for details.

#include "base/clock.h"

#include <time.h>
#include <cstring>
#include <stdexcept>
#include <system_error>

#include "base/duration.h"
#include "base/time.h"

namespace base {

void Clock::assert_valid() const {
  if (!ptr_) throw std::logic_error("base::Clock is empty");
}

class SystemClock : public ClockImpl {
 public:
  explicit SystemClock(clockid_t id) noexcept : id_(id) {}

  Time now() const override;

 private:
  clockid_t id_;
};

Time SystemClock::now() const {
  struct timespec ts;
  ::bzero(&ts, sizeof(ts));
  int rc = clock_gettime(id_, &ts);
  if (rc != 0) {
    int err_no = errno;
    throw std::system_error(err_no, std::system_category(), "clock_gettime(2)");
  }
  return Time::from_epoch(Duration::raw(false, ts.tv_sec, ts.tv_nsec));
}

Clock& system_wallclock() {
  static auto& ref = *new Clock(std::make_shared<SystemClock>(CLOCK_REALTIME));
  return ref;
}

Clock& system_monotonic_clock() {
  static auto& ref = *new Clock(std::make_shared<SystemClock>(CLOCK_MONOTONIC));
  return ref;
}

}  // namespace base
