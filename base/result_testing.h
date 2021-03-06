// base/result_testing.h - Macros for checking base::Result values in tests
// Copyright © 2016 by Donald King <chronos@chronos-tachyon.net>
// Available under the MIT License. See LICENSE for details.

#ifndef BASE_RESULT_TESTING_H
#define BASE_RESULT_TESTING_H

#include "base/result.h"
#include "gtest/gtest.h"

namespace base {
namespace testing {

// Implementation {{{

inline ::testing::AssertionResult ResultCodeEQ(const char* code_text,
                                               const char* expr_text,
                                               ResultCode code,
                                               const Result& expr) {
  auto cast = [](ResultCode code) {
    return static_cast<uint16_t>(static_cast<uint8_t>(code));
  };
  if (code == expr.code()) return ::testing::AssertionSuccess();
  return ::testing::AssertionFailure() << "expression: " << expr_text << "\n"
                                       << "  expected: " << code << "("
                                       << cast(code) << ")\n"
                                       << "       got: " << expr.as_string();
}

// }}}
// Assertions {{{

#define ASSERT_OK(x) \
  ASSERT_PRED_FORMAT2(::base::testing::ResultCodeEQ, ::base::ResultCode::OK, x)

#define ASSERT_UNKNOWN(x)                            \
  ASSERT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::UNKNOWN, x)

#define ASSERT_INTERNAL(x)                           \
  ASSERT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::INTERNAL, x)

#define ASSERT_CANCELLED(x)                          \
  ASSERT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::CANCELLED, x)

#define ASSERT_FAILED_PRECONDITION(x)                \
  ASSERT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::FAILED_PRECONDITION, x)

#define ASSERT_NOT_FOUND(x)                          \
  ASSERT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::NOT_FOUND, x)

#define ASSERT_ALREADY_EXISTS(x)                     \
  ASSERT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::ALREADY_EXISTS, x)

#define ASSERT_WRONG_TYPE(x)                         \
  ASSERT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::WRONG_TYPE, x)

#define ASSERT_PERMISSION_DENIED(x)                  \
  ASSERT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::PERMISSION_DENIED, x)

#define ASSERT_UNAUTHENTICATED(x)                    \
  ASSERT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::UNAUTHENTICATED, x)

#define ASSERT_INVALID_ARGUMENT(x)                   \
  ASSERT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::INVALID_ARGUMENT, x)

#define ASSERT_OUT_OF_RANGE(x)                       \
  ASSERT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::OUT_OF_RANGE, x)

#define ASSERT_NOT_IMPLEMENTED(x)                    \
  ASSERT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::NOT_IMPLEMENTED, x)

#define ASSERT_UNAVAILABLE(x)                        \
  ASSERT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::UNAVAILABLE, x)

#define ASSERT_ABORTED(x)                            \
  ASSERT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::ABORTED, x)

#define ASSERT_RESOURCE_EXHAUSTED(x)                 \
  ASSERT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::RESOURCE_EXHAUSTED, x)

#define ASSERT_DEADLINE_EXCEEDED(x)                  \
  ASSERT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::DEADLINE_EXCEEDED, x)

#define ASSERT_DATA_LOSS(x)                          \
  ASSERT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::DATA_LOSS, x)

#define ASSERT_EOF(x)                                \
  ASSERT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::END_OF_FILE, x)

// }}}
// Expectations {{{

#define EXPECT_OK(x) \
  EXPECT_PRED_FORMAT2(::base::testing::ResultCodeEQ, ::base::ResultCode::OK, x)

#define EXPECT_UNKNOWN(x)                            \
  EXPECT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::UNKNOWN, x)

#define EXPECT_INTERNAL(x)                           \
  EXPECT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::INTERNAL, x)

#define EXPECT_CANCELLED(x)                          \
  EXPECT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::CANCELLED, x)

#define EXPECT_FAILED_PRECONDITION(x)                \
  EXPECT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::FAILED_PRECONDITION, x)

#define EXPECT_NOT_FOUND(x)                          \
  EXPECT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::NOT_FOUND, x)

#define EXPECT_ALREADY_EXISTS(x)                     \
  EXPECT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::ALREADY_EXISTS, x)

#define EXPECT_WRONG_TYPE(x)                         \
  EXPECT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::WRONG_TYPE, x)

#define EXPECT_PERMISSION_DENIED(x)                  \
  EXPECT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::PERMISSION_DENIED, x)

#define EXPECT_UNAUTHENTICATED(x)                    \
  EXPECT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::UNAUTHENTICATED, x)

#define EXPECT_INVALID_ARGUMENT(x)                   \
  EXPECT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::INVALID_ARGUMENT, x)

#define EXPECT_OUT_OF_RANGE(x)                       \
  EXPECT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::OUT_OF_RANGE, x)

#define EXPECT_NOT_IMPLEMENTED(x)                    \
  EXPECT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::NOT_IMPLEMENTED, x)

#define EXPECT_UNAVAILABLE(x)                        \
  EXPECT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::UNAVAILABLE, x)

#define EXPECT_ABORTED(x)                            \
  EXPECT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::ABORTED, x)

#define EXPECT_RESOURCE_EXHAUSTED(x)                 \
  EXPECT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::RESOURCE_EXHAUSTED, x)

#define EXPECT_DEADLINE_EXCEEDED(x)                  \
  EXPECT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::DEADLINE_EXCEEDED, x)

#define EXPECT_DATA_LOSS(x)                          \
  EXPECT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::DATA_LOSS, x)

#define EXPECT_EOF(x)                                \
  EXPECT_PRED_FORMAT2(::base::testing::ResultCodeEQ, \
                      ::base::ResultCode::END_OF_FILE, x)

// }}}

}  // namespace testing
}  // namespace base

#endif  // BASE_RESULT_TESTING_H
