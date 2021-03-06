# https://bazel.build/
# vim: set ft=python sts=2 sw=2 et:

cc_library(
  name = "event",
  srcs = [
    "callback.cc",
    "data.cc",
    "dispatcher.cc",
    "handler.cc",
    "manager.cc",
    "poller.cc",
    "set.cc",
    "task.cc",
  ],
  hdrs = [
    "callback.h",
    "data.h",
    "dispatcher.h",
    "handler.h",
    "manager.h",
    "poller.h",
    "set.h",
    "task.h",
  ],
  deps = [
    "//base",
    "//base/time",
  ],
  linkopts = ["-pthread"],
  visibility = ["//visibility:public"],
)

cc_test(
  name = "callback_test",
  srcs = ["callback_test.cc"],
  deps = [
    ":event",
    "//base",
    "//base:result_testing",
    "//external:gtest",
  ],
  size = "small",
  timeout = "short",
)

cc_test(
  name = "dispatcher_test",
  srcs = ["dispatcher_test.cc"],
  deps = [
    ":event",
    "//base",
    "//base:result_testing",
    "//external:gtest",
  ],
  size = "small",
  timeout = "short",
)

cc_test(
  name = "handler_test",
  srcs = ["handler_test.cc"],
  deps = [
    ":event",
    "//base",
    "//base:result_testing",
    "//external:gtest",
  ],
  size = "small",
  timeout = "short",
)

cc_test(
  name = "manager_test",
  srcs = ["manager_test.cc"],
  deps = [
    ":event",
    "//base",
    "//base:result_testing",
    "//external:gtest",
  ],
  size = "small",
  timeout = "short",
)

cc_test(
  name = "poller_test",
  srcs = ["poller_test.cc"],
  deps = [
    ":event",
    "//base",
    "//base:result_testing",
    "//external:gtest",
  ],
  size = "small",
  timeout = "short",
)

cc_test(
  name = "task_test",
  srcs = ["task_test.cc"],
  deps = [
    ":event",
    "//base",
    "//base:result_testing",
    "//external:gtest",
  ],
  size = "small",
  timeout = "short",
)
