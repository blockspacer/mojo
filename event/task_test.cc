// Copyright © 2016 by Donald King <chronos@chronos-tachyon.net>
// Available under the MIT License. See LICENSE for details.

#include "gtest/gtest.h"

#include <iostream>

#include "base/logging.h"
#include "base/result_testing.h"
#include "event/task.h"

TEST(Task, Inline) {
  int m = 0, n = 0;
  auto inc_m = [&m] {
    ++m;
    return base::Result();
  };
  auto inc_n = [&n] {
    ++n;
    return base::Result();
  };

  LOG(INFO) << "1. create task";
  event::Task task;
  task.on_cancelled(event::callback(inc_m));
  task.on_finished(event::callback(inc_n));
  EXPECT_EQ(event::Task::State::ready, task.state());
  EXPECT_FALSE(task.is_finished());
  EXPECT_EQ(0, m);
  EXPECT_EQ(0, n);

  LOG(INFO) << "1. start task";
  EXPECT_TRUE(task.start());
  EXPECT_EQ(event::Task::State::running, task.state());
  EXPECT_FALSE(task.is_finished());
  EXPECT_EQ(0, m);
  EXPECT_EQ(0, n);

  LOG(INFO) << "1. finish task [OK]";
  task.finish_ok();
  EXPECT_EQ(event::Task::State::done, task.state());
  EXPECT_TRUE(task.is_finished());
  EXPECT_OK(task.result());
  EXPECT_EQ(0, m);
  EXPECT_EQ(1, n);

  LOG(INFO) << "1. on_cancelled after finish";
  task.on_cancelled(event::callback(inc_m));
  EXPECT_EQ(0, m);

  LOG(INFO) << "1. on_finished after finish";
  task.on_finished(event::callback(inc_n));
  EXPECT_EQ(2, n);

  LOG(INFO) << "2. reset task";
  m = n = 0;
  task.reset();
  task.on_cancelled(event::callback(inc_m));
  task.on_finished(event::callback(inc_n));
  EXPECT_EQ(event::Task::State::ready, task.state());
  EXPECT_EQ(0, m);
  EXPECT_EQ(0, n);

  LOG(INFO) << "2. cancel task";
  task.cancel();
  EXPECT_EQ(event::Task::State::unstarted, task.state());

  LOG(INFO) << "2. start task";
  EXPECT_FALSE(task.start());
  EXPECT_EQ(event::Task::State::done, task.state());
  EXPECT_TRUE(task.is_finished());
  EXPECT_CANCELLED(task.result());
  EXPECT_EQ(1, m);
  EXPECT_EQ(1, n);

  LOG(INFO) << "2. on_cancelled after cancel";
  task.on_cancelled(event::callback(inc_m));
  EXPECT_EQ(2, m);

  LOG(INFO) << "2. on_finished after cancel";
  task.on_finished(event::callback(inc_n));
  EXPECT_EQ(2, n);

  LOG(INFO) << "3. reset task";
  m = n = 0;
  task.reset();
  task.on_cancelled(event::callback(inc_m));
  task.on_finished(event::callback(inc_n));
  EXPECT_EQ(event::Task::State::ready, task.state());
  EXPECT_EQ(0, m);
  EXPECT_EQ(0, n);

  LOG(INFO) << "3. start task";
  EXPECT_TRUE(task.start());
  EXPECT_EQ(0, m);
  EXPECT_EQ(0, n);

  LOG(INFO) << "3. cancel task";
  task.cancel();
  EXPECT_EQ(event::Task::State::cancelling, task.state());
  EXPECT_FALSE(task.is_finished());
  EXPECT_EQ(1, m);
  EXPECT_EQ(0, n);

  LOG(INFO) << "3. on_cancelled after cancel";
  task.on_cancelled(event::callback(inc_m));
  EXPECT_EQ(2, m);

  LOG(INFO) << "3. finish task [CANCELLED]";
  task.finish_cancel();
  EXPECT_EQ(event::Task::State::done, task.state());
  EXPECT_TRUE(task.is_finished());
  EXPECT_CANCELLED(task.result());
  EXPECT_EQ(2, m);
  EXPECT_EQ(1, n);

  LOG(INFO) << "3. on_cancelled after finish";
  task.on_cancelled(event::callback(inc_m));
  EXPECT_EQ(3, m);

  LOG(INFO) << "3. on_finished after finish";
  task.on_finished(event::callback(inc_n));
  EXPECT_EQ(2, n);

  LOG(INFO) << "4. reset task";
  m = n = 0;
  task.reset();
  task.on_cancelled(event::callback(inc_m));
  task.on_finished(event::callback(inc_n));
  EXPECT_EQ(event::Task::State::ready, task.state());
  EXPECT_EQ(0, m);
  EXPECT_EQ(0, n);

  LOG(INFO) << "4. expire task";
  task.expire();
  EXPECT_EQ(event::Task::State::unstarted, task.state());

  LOG(INFO) << "4. start task";
  EXPECT_FALSE(task.start());
  EXPECT_EQ(event::Task::State::done, task.state());
  EXPECT_TRUE(task.is_finished());
  EXPECT_DEADLINE_EXCEEDED(task.result());
  EXPECT_EQ(1, m);
  EXPECT_EQ(1, n);

  LOG(INFO) << "4. on_cancelled after expire";
  task.on_cancelled(event::callback(inc_m));
  EXPECT_EQ(2, m);

  LOG(INFO) << "4. on_finished after expire";
  task.on_finished(event::callback(inc_n));
  EXPECT_EQ(2, n);

  LOG(INFO) << "5. reset task";
  m = n = 0;
  task.reset();
  task.on_cancelled(event::callback(inc_m));
  task.on_finished(event::callback(inc_n));
  EXPECT_EQ(event::Task::State::ready, task.state());
  EXPECT_EQ(0, m);
  EXPECT_EQ(0, n);

  LOG(INFO) << "5. start task";
  EXPECT_TRUE(task.start());
  EXPECT_EQ(0, m);
  EXPECT_EQ(0, n);

  LOG(INFO) << "5. expire task";
  task.expire();
  EXPECT_EQ(event::Task::State::expiring, task.state());
  EXPECT_FALSE(task.is_finished());
  EXPECT_EQ(1, m);
  EXPECT_EQ(0, n);

  LOG(INFO) << "5. on_cancelled after expire";
  task.on_cancelled(event::callback(inc_m));
  EXPECT_EQ(2, m);

  LOG(INFO) << "5. finish task [DEADLINE_EXCEEDED]";
  task.finish_cancel();
  EXPECT_EQ(event::Task::State::done, task.state());
  EXPECT_TRUE(task.is_finished());
  EXPECT_DEADLINE_EXCEEDED(task.result());
  EXPECT_EQ(2, m);
  EXPECT_EQ(1, n);

  LOG(INFO) << "5. on_cancelled after finish";
  task.on_cancelled(event::callback(inc_m));
  EXPECT_EQ(3, m);

  LOG(INFO) << "5. on_finished after finish";
  task.on_finished(event::callback(inc_n));
  EXPECT_EQ(2, n);

  LOG(INFO) << "6. reset task";
  m = n = 0;
  task.reset();
  task.on_cancelled(event::callback(inc_m));
  task.on_finished(event::callback(inc_n));
  EXPECT_EQ(event::Task::State::ready, task.state());
  EXPECT_EQ(0, m);
  EXPECT_EQ(0, n);

  LOG(INFO) << "6. start task";
  EXPECT_TRUE(task.start());
  EXPECT_EQ(0, m);
  EXPECT_EQ(0, n);

  LOG(INFO) << "6. expire task";
  task.expire();
  EXPECT_EQ(event::Task::State::expiring, task.state());
  EXPECT_FALSE(task.is_finished());
  EXPECT_EQ(1, m);
  EXPECT_EQ(0, n);

  LOG(INFO) << "6. cancel task";
  task.cancel();
  EXPECT_EQ(event::Task::State::cancelling, task.state());
  EXPECT_FALSE(task.is_finished());
  EXPECT_EQ(1, m);
  EXPECT_EQ(0, n);

  LOG(INFO) << "6. expire task again";
  task.expire();
  EXPECT_EQ(event::Task::State::cancelling, task.state());
  EXPECT_FALSE(task.is_finished());
  EXPECT_EQ(1, m);
  EXPECT_EQ(0, n);

  LOG(INFO) << "6. finish task [CANCELLED]";
  task.finish_cancel();
  EXPECT_EQ(event::Task::State::done, task.state());
  EXPECT_TRUE(task.is_finished());
  EXPECT_CANCELLED(task.result());
  EXPECT_EQ(1, m);
  EXPECT_EQ(1, n);
}

TEST(Task, SubtaskCancel) {
  event::Task parent;
  EXPECT_TRUE(parent.start());

  event::Task child0, child1;
  parent.add_subtask(&child0);
  parent.add_subtask(&child1);
  EXPECT_TRUE(child0.start());
  EXPECT_TRUE(child1.start());

  child0.finish_ok();
  parent.cancel();

  EXPECT_EQ(event::Task::State::cancelling, parent.state());
  EXPECT_EQ(event::Task::State::done, child0.state());
  EXPECT_EQ(event::Task::State::cancelling, child1.state());

  child1.finish_cancel();
  parent.finish_cancel();

  EXPECT_EQ(event::Task::State::done, parent.state());
  EXPECT_EQ(event::Task::State::done, child0.state());
  EXPECT_EQ(event::Task::State::done, child1.state());
}

TEST(Task, SubtaskExpire) {
  event::Task parent;
  EXPECT_TRUE(parent.start());

  event::Task child0, child1;
  parent.add_subtask(&child0);
  parent.add_subtask(&child1);
  EXPECT_TRUE(child0.start());
  EXPECT_TRUE(child1.start());

  child0.finish_ok();
  parent.expire();

  EXPECT_EQ(event::Task::State::expiring, parent.state());
  EXPECT_EQ(event::Task::State::done, child0.state());
  EXPECT_EQ(event::Task::State::cancelling, child1.state());

  child1.finish_cancel();
  parent.finish_cancel();

  EXPECT_EQ(event::Task::State::done, parent.state());
  EXPECT_EQ(event::Task::State::done, child0.state());
  EXPECT_EQ(event::Task::State::done, child1.state());
}
