// Copyright © 2016 by Donald King <chronos@chronos-tachyon.net>
// Available under the MIT License. See LICENSE for details.

#include "base/logging.h"

#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "base/concat.h"
#include "base/debug.h"
#include "base/result.h"
#include "base/util.h"

namespace base {

static pid_t my_gettid() { return syscall(SYS_gettid); }

namespace {
struct Key {
  const char* file;
  unsigned int line;

  Key(const char* file, unsigned int line) noexcept : file(file), line(line) {}
};

static bool operator==(Key a, Key b) noexcept __attribute__((unused));
static bool operator==(Key a, Key b) noexcept {
  return a.line == b.line && ::strcmp(a.file, b.file) == 0;
}

static bool operator<(Key a, Key b) noexcept {
  int cmp = ::strcmp(a.file, b.file);
  return cmp < 0 || (cmp == 0 && a.line < b.line);
}
}  // anonymous namespace

using Map = std::map<Key, std::size_t>;
using Vec = std::vector<LogTarget*>;
using Queue = std::deque<LogEntry>;

enum ThreadState {
  kThreadNotStarted = 0,
  kThreadStarted = 1,
  kSingleThreaded = 2,
};

static std::once_flag g_once;
static std::mutex g_mu;
static std::mutex g_queue_mu;
static level_t g_stderr = 0;                      // protected by g_mu
static GetTidFunc g_gtid = nullptr;               // protected by g_mu
static GetTimeOfDayFunc g_gtod = nullptr;         // protected by g_mu
static Map* g_map = nullptr;                      // protected by g_mu
static Vec* g_vec = nullptr;                      // protected by g_mu
static int g_thread_state = kThreadNotStarted;    // protected by g_queue_mu
static std::condition_variable g_queue_put_cv;    // protected by g_queue_mu
static std::condition_variable g_queue_empty_cv;  // protected by g_queue_mu
static Queue* g_queue = nullptr;                  // protected by g_queue_mu

static base::LogTarget* make_stderr();

static void thread_body();

static void init() {
  auto queue_lock = acquire_lock(g_queue_mu);
  auto main_lock = acquire_lock(g_mu);

  if (!g_stderr) g_stderr = LOG_LEVEL_INFO;
  if (!g_gtid) g_gtid = my_gettid;
  if (!g_gtod) g_gtod = gettimeofday;
  if (!g_map) g_map = new Map;
  if (!g_vec) g_vec = new Vec{make_stderr()};
  if (!g_queue) g_queue = new Queue;

  if (g_thread_state == kThreadNotStarted) {
    g_thread_state = kThreadStarted;
    std::thread(thread_body).detach();
  }
}

static void process(base::Lock& main_lock, LogEntry entry) {
  for (LogTarget* target : *g_vec) {
    if (target->want(entry.file, entry.line, entry.level)) {
      target->log(entry);
    }
  }
}

static void thread_body() {
  auto queue_lock = base::acquire_lock(g_queue_mu);
  while (true) {
    if (g_queue->empty()) {
      g_queue_empty_cv.notify_all();
      while (g_queue->empty()) g_queue_put_cv.wait(queue_lock);
    }
    auto entry = std::move(g_queue->front());
    g_queue->pop_front();
    auto main_lock = base::acquire_lock(g_mu);
    process(main_lock, std::move(entry));
  }
}

static bool want(const char* file, unsigned int line, unsigned int n,
                 level_t level) {
  std::call_once(g_once, [] { init(); });
  if (level >= LOG_LEVEL_DFATAL) return true;
  auto main_lock = base::acquire_lock(g_mu);
  if (n > 1) {
    Key key(file, line);
    auto pair = g_map->insert(std::make_pair(key, 0));
    auto& count = pair.first->second;
    bool x = (count == 0);
    count = (count + 1) % n;
    if (!x) return false;
  }
  for (const auto& target : *g_vec) {
    if (target->want(file, line, level)) return true;
  }
  return false;
}

static void log(LogEntry entry) {
  std::call_once(g_once, [] { init(); });
  auto queue_lock = base::acquire_lock(g_queue_mu);
  if (g_thread_state == kSingleThreaded) {
    auto main_lock = base::acquire_lock(g_mu);
    process(main_lock, std::move(entry));
    return;
  }
  g_queue->push_back(std::move(entry));
  g_queue_put_cv.notify_one();
}

namespace {
class LogSTDERR : public LogTarget {
 public:
  LogSTDERR() noexcept = default;
  bool want(const char* file, unsigned int line, level_t level) const
      noexcept override {
    // g_mu held by base::want()
    return level >= g_stderr;
  }
  void log(const LogEntry& entry) noexcept override {
    // g_mu held by base::thread_body()
    auto str = entry.as_string();
    ::write(2, str.data(), str.size());
  }
};
}  // anonymous namespace

static base::LogTarget* make_stderr() {
  static base::LogTarget* const ptr = new LogSTDERR;
  return ptr;
}

LogEntry::LogEntry(const char* file, unsigned int line, level_t level,
                   std::string message) noexcept : file(file),
                                                   line(line),
                                                   level(level),
                                                   message(std::move(message)) {
  std::call_once(g_once, [] { init(); });
  auto main_lock = acquire_lock(g_mu);
  ::bzero(&time, sizeof(time));
  (*g_gtod)(&time, nullptr);
  tid = (*g_gtid)();
}

void LogEntry::append_to(std::string& out) const {
  char ch;
  if (level >= LOG_LEVEL_DFATAL) {
    ch = 'F';
  } else if (level >= LOG_LEVEL_ERROR) {
    ch = 'E';
  } else if (level >= LOG_LEVEL_WARN) {
    ch = 'W';
  } else if (level >= LOG_LEVEL_INFO) {
    ch = 'I';
  } else {
    ch = 'D';
  }

  struct tm tm;
  ::gmtime_r(&time.tv_sec, &tm);

  // "[IWEF]<mm><dd> <hh>:<mm>:<ss>.<uuuuuu>  <tid> <file>:<line>] <message>"

  std::array<char, 24> buf;
  ::snprintf(buf.data(), buf.size(), "%c%02u%02u %02u:%02u:%02u.%06lu  ", ch,
             tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
             time.tv_usec);
  concat_to(out, std::string(buf.data()), tid, ' ', file, ':', line, "] ",
            message, '\n');
}

std::string LogEntry::as_string() const {
  std::string out;
  append_to(out);
  return out;
}

static std::unique_ptr<std::ostringstream> make_ss(const char* file,
                                                   unsigned int line,
                                                   unsigned int n,
                                                   level_t level) {
  std::unique_ptr<std::ostringstream> ptr;
  if (want(file, line, n, level)) ptr.reset(new std::ostringstream);
  return ptr;
}

Logger::Logger(std::nullptr_t)
    : file_(nullptr), line_(0), n_(0), level_(0), ss_(nullptr) {}

Logger::Logger(const char* file, unsigned int line, unsigned int every_n,
               level_t level)
    : file_(file),
      line_(line),
      n_(every_n),
      level_(level),
      ss_(make_ss(file, line, every_n, level)) {}

Logger::~Logger() noexcept(false) {
  if (ss_) {
    log(LogEntry(file_, line_, level_, ss_->str()));
    if (level_ >= LOG_LEVEL_DFATAL) {
      log_flush();
      if (level_ >= LOG_LEVEL_FATAL || debug()) {
        std::terminate();
      }
    }
  }
}

void log_set_gettid(GetTidFunc func) {
  auto main_lock = acquire_lock(g_mu);
  if (func)
    g_gtid = func;
  else
    g_gtid = my_gettid;
}

void log_set_gettimeofday(GetTimeOfDayFunc func) {
  auto main_lock = acquire_lock(g_mu);
  if (func)
    g_gtod = func;
  else
    g_gtod = gettimeofday;
}

void log_single_threaded() {
  auto queue_lock = acquire_lock(g_queue_mu);
  if (g_thread_state == kThreadStarted)
    throw std::logic_error("logging thread is already running!");
  g_thread_state = kSingleThreaded;
}

void log_flush() {
  auto queue_lock = base::acquire_lock(g_queue_mu);
  while (!g_queue->empty()) g_queue_empty_cv.wait(queue_lock);
}

void log_stderr_set_level(level_t level) {
  auto main_lock = acquire_lock(g_mu);
  g_stderr = level;
}

void log_target_add(LogTarget* target) {
  auto main_lock = acquire_lock(g_mu);
  g_vec->push_back(target);
}

void log_target_remove(LogTarget* target) {
  auto queue_lock = base::acquire_lock(g_queue_mu);
  while (!g_queue->empty()) g_queue_empty_cv.wait(queue_lock);
  auto main_lock = acquire_lock(g_mu);
  auto& v = *g_vec;
  for (auto it = v.begin(), end = v.end(); it != end; ++it) {
    if (*it == target) {
      v.erase(it);
      break;
    }
  }
}

void log_exception(const char* file, unsigned int line, std::exception_ptr e) {
  Logger logger(file, line, 1, LOG_LEVEL_ERROR);
  try {
    std::rethrow_exception(e);
  } catch (const std::system_error& e) {
    const auto& ecode = e.code();
    logger << "caught std::system_error\n"
           << "\t" << ecode.category().name() << "(" << ecode.value()
           << "): " << e.what();
  } catch (const std::exception& e) {
    logger << "caught std::exception\n"
           << "\t[" << typeid(e).name() << "]\n"
           << "\t" << e.what();
  } catch (...) {
    logger << "caught unclassifiable exception!";
  }
}

Logger log_check(const char* file, unsigned int line, const char* expr,
                 bool cond) {
  if (cond) return Logger(nullptr);
  Logger logger(file, line, 1, LOG_LEVEL_DFATAL);
  logger << "CHECK FAILED: " << expr;
  return logger;
}

Logger log_check_ok(const char* file, unsigned int line, const char* expr,
                    const Result& rslt) {
  if (rslt) return Logger(nullptr);
  Logger logger(file, line, 1, LOG_LEVEL_DFATAL);
  logger << "CHECK FAILED: " << expr << ": " << rslt.as_string();
  return logger;
}

Logger force_eval(bool) { return Logger(nullptr); }

}  // namespace base
