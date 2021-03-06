// Copyright © 2017 by Donald King <chronos@chronos-tachyon.net>
// Available under the MIT License. See LICENSE for details.

#include "gtest/gtest.h"

#include "base/strings.h"

using SP = base::StringPiece;

static std::string S(std::string str) noexcept { return std::move(str); }

static std::string S(const std::vector<SP>& vec) {
  std::string out;
  out.push_back('[');

  bool first = true;
  for (SP item : vec) {
    if (first)
      first = false;
    else
      out.push_back(' ');
    out.push_back('"');
    item.append_to(&out);
    out.push_back('"');
  }
  out.push_back(']');
  return out;
}

static testing::AssertionResult vec_eq(const char* lhs_expr,
                                       const char* rhs_expr,
                                       const std::vector<SP>& lhs,
                                       const std::vector<SP>& rhs) {
  bool eq = (lhs.size() == rhs.size());
  if (eq) {
    for (std::size_t i = 0, n = lhs.size(); i < n; ++i) {
      if (lhs[i] != rhs[i]) {
        eq = false;
        break;
      }
    }
  }
  if (eq) return testing::AssertionSuccess();
  return testing::AssertionFailure() << lhs_expr << " differs from " << rhs_expr
                                     << "\n"
                                     << "expected: " << S(lhs) << "\n"
                                     << "  actual: " << S(rhs);
}

TEST(StringPiece, Construct) {
  constexpr const char* kHello = "Hello!";

  constexpr SP kEmpty;
  static_assert(kEmpty.size() == 0, "");
  static_assert(kEmpty.empty(), "");

  constexpr SP kEmptyCopy(kEmpty);
  static_assert(kEmptyCopy.data() == kEmpty.data(), "");
  static_assert(kEmptyCopy.size() == 0, "");
  static_assert(kEmptyCopy.empty(), "");

  constexpr SP kPtrLen(kHello, 6);
  static_assert(kPtrLen.data() == kHello, "");
  static_assert(kPtrLen.size() == 6, "");
  static_assert(!kPtrLen.empty(), "");

  constexpr SP kPtr(kHello);
  static_assert(kPtr.data() == kHello, "");
  static_assert(kPtr.size() == 6, "");
  static_assert(!kPtr.empty(), "");

  constexpr SP kConst("Hello!");
  static_assert(kConst.data() != nullptr, "");
  static_assert(kConst.size() == 6, "");
  static_assert(!kConst.empty(), "");
  static_assert(kConst.front() == 'H', "");
  static_assert(kConst.back() == '!', "");
  static_assert(kConst[1] == 'e', "");

  std::string str("Hello!");
  const SP strsp(str);
  EXPECT_EQ(str.data(), strsp.data());
  EXPECT_EQ(str.size(), strsp.size());
  EXPECT_FALSE(strsp.empty());

  std::vector<char> vec;
  vec.insert(vec.end(), str.begin(), str.end());
  const SP vecsp(vec);
  EXPECT_EQ(vec.data(), vecsp.data());
  EXPECT_EQ(vec.size(), vecsp.size());
  EXPECT_FALSE(vecsp.empty());
}

TEST(StringPiece, Compare) {
  constexpr SP f = "f";
  constexpr SP foo = "foo";
  constexpr SP fooo = "fooo";
  constexpr SP fop = "fop";
  constexpr SP g = "g";

#define LT(a, b)                       \
  static_assert(a.compare(b) < 0, ""); \
  static_assert(a != b, "");           \
  static_assert(a < b, "");            \
  static_assert(a <= b, "");

#define EQ(a, b)                        \
  static_assert(a.compare(b) == 0, ""); \
  static_assert(a == b, "");            \
  static_assert(a <= b, "");            \
  static_assert(a >= b, "");

#define GT(a, b)                       \
  static_assert(a.compare(b) > 0, ""); \
  static_assert(a != b, "");           \
  static_assert(a > b, "");            \
  static_assert(a >= b, "");

  EQ(f, f);

  LT(f, foo);
  EQ(foo, foo);
  GT(foo, f);

  LT(f, fooo);
  LT(foo, fooo);
  EQ(fooo, fooo);
  GT(fooo, foo);
  GT(fooo, f);

  LT(f, fop);
  LT(foo, fop);
  LT(fooo, fop);
  EQ(fop, fop);
  GT(fop, fooo);
  GT(fop, foo);
  GT(fop, f);

  LT(f, g);
  LT(foo, g);
  LT(fooo, g);
  LT(fop, g);
  EQ(g, g);
  GT(g, fop);
  GT(g, fooo);
  GT(g, foo);
  GT(g, f);

#undef GT
#undef EQ
#undef LT
}

TEST(StringPiece, Substring) {
  constexpr SP foo = "abcdefghi";
  static_assert(foo.substring(0, 3) == "abc", "");
  static_assert(foo.substring(3, 3) == "def", "");
  static_assert(foo.substring(6, 3) == "ghi", "");
  static_assert(foo.substring(8, 3) == "i", "");
  static_assert(foo.substring(9, 3) == "", "");
  static_assert(foo.substring(10, 3) == "", "");

  static_assert(foo.substring(0) == "abcdefghi", "");
  static_assert(foo.substring(3) == "defghi", "");
  static_assert(foo.substring(6) == "ghi", "");
  static_assert(foo.substring(8) == "i", "");
  static_assert(foo.substring(9) == "", "");
  static_assert(foo.substring(10) == "", "");

  static_assert(foo.prefix(0) == "", "");
  static_assert(foo.prefix(1) == "a", "");
  static_assert(foo.prefix(3) == "abc", "");

  static_assert(foo.suffix(0) == "", "");
  static_assert(foo.suffix(1) == "i", "");
  static_assert(foo.suffix(3) == "ghi", "");

  static_assert(foo.has_prefix(""), "");
  static_assert(foo.has_prefix("a"), "");
  static_assert(foo.has_prefix("abc"), "");
  static_assert(!foo.has_prefix("x"), "");

  static_assert(foo.has_suffix(""), "");
  static_assert(foo.has_suffix("i"), "");
  static_assert(foo.has_suffix("ghi"), "");
  static_assert(!foo.has_suffix("x"), "");

  static_assert(foo.strip_prefix(0) == "abcdefghi", "");
  static_assert(foo.strip_prefix(1) == "bcdefghi", "");
  static_assert(foo.strip_prefix(3) == "defghi", "");
  static_assert(foo.strip_prefix(8) == "i", "");
  static_assert(foo.strip_prefix(9) == "", "");
  static_assert(foo.strip_prefix(10) == "", "");

  static_assert(foo.strip_prefix("") == "abcdefghi", "");
  static_assert(foo.strip_prefix("a") == "bcdefghi", "");
  static_assert(foo.strip_prefix("abc") == "defghi", "");
  static_assert(foo.strip_prefix("x") == "abcdefghi", "");

  static_assert(foo.strip_suffix(0) == "abcdefghi", "");
  static_assert(foo.strip_suffix(1) == "abcdefgh", "");
  static_assert(foo.strip_suffix(3) == "abcdef", "");
  static_assert(foo.strip_suffix(8) == "a", "");
  static_assert(foo.strip_suffix(9) == "", "");
  static_assert(foo.strip_suffix(10) == "", "");

  static_assert(foo.strip_suffix("") == "abcdefghi", "");
  static_assert(foo.strip_suffix("i") == "abcdefgh", "");
  static_assert(foo.strip_suffix("ghi") == "abcdef", "");
  static_assert(foo.strip_suffix("x") == "abcdefghi", "");

  SP sp = foo;
  EXPECT_FALSE(sp.remove_prefix("xyz"));
  EXPECT_EQ("abcdefghi", sp);

  sp = foo;
  EXPECT_TRUE(sp.remove_prefix("abc"));
  EXPECT_EQ("defghi", sp);

  sp = foo;
  sp.remove_prefix(3);
  EXPECT_EQ("defghi", sp);

  sp = foo;
  sp.remove_prefix(100);
  EXPECT_EQ("", sp);

  sp = foo;
  EXPECT_FALSE(sp.remove_suffix("xyz"));
  EXPECT_EQ("abcdefghi", sp);

  sp = foo;
  EXPECT_TRUE(sp.remove_suffix("ghi"));
  EXPECT_EQ("abcdef", sp);

  sp = foo;
  sp.remove_suffix(3);
  EXPECT_EQ("abcdef", sp);

  sp = foo;
  sp.remove_suffix(100);
  EXPECT_EQ("", sp);
}

TEST(StringPiece, Find) {
  constexpr SP abc = "a,b,c";

  static_assert(abc.find('a') == 0, "");
  static_assert(abc.find('a', 1) == SP::npos, "");

  static_assert(abc.find(',') == 1, "");
  static_assert(abc.find(',', 1) == 1, "");
  static_assert(abc.find(',', 2) == 3, "");
  static_assert(abc.find(',', 3) == 3, "");
  static_assert(abc.find(',', 4) == SP::npos, "");

  static_assert(abc.rfind('a') == 0, "");
  static_assert(abc.rfind('a', 1) == 0, "");

  static_assert(abc.rfind(',') == 3, "");
  static_assert(abc.rfind(',', 3) == 3, "");
  static_assert(abc.rfind(',', 2) == 1, "");
  static_assert(abc.rfind(',', 1) == 1, "");
  static_assert(abc.rfind(',', 0) == SP::npos, "");

  constexpr SP foo = "foo,bar,baz";

  static_assert(foo.find("foo") == 0, "");
  static_assert(foo.find("foo", 1) == SP::npos, "");
  static_assert(foo.find("bar") == 4, "");
  static_assert(foo.find("bar", 3) == 4, "");
  static_assert(foo.find("bar", 4) == 4, "");
  static_assert(foo.find("bar", 5) == SP::npos, "");
  static_assert(foo.find("baz") == 8, "");
  static_assert(foo.find("baz", 7) == 8, "");
  static_assert(foo.find("baz", 8) == 8, "");
  static_assert(foo.find("baz", 9) == SP::npos, "");
  static_assert(foo.find("baz", 1000) == SP::npos, "");
  static_assert(foo.find("x") == SP::npos, "");

  static_assert(foo.rfind("foo") == 0, "");
  static_assert(foo.rfind("foo", 3) == 0, "");
  static_assert(foo.rfind("foo", 2) == 0, "");
  static_assert(foo.rfind("foo", 1) == 0, "");
  static_assert(foo.rfind("foo", 0) == 0, "");
  static_assert(foo.rfind("bar") == 4, "");
  static_assert(foo.rfind("bar", 7) == 4, "");
  static_assert(foo.rfind("bar", 6) == 4, "");
  static_assert(foo.rfind("bar", 5) == 4, "");
  static_assert(foo.rfind("bar", 4) == 4, "");
  static_assert(foo.rfind("bar", 3) == SP::npos, "");
  static_assert(foo.rfind("baz") == 8, "");
  static_assert(foo.rfind("baz", 1000) == 8, "");
  static_assert(foo.rfind("baz", 11) == 8, "");
  static_assert(foo.rfind("baz", 10) == 8, "");
  static_assert(foo.rfind("baz", 9) == 8, "");
  static_assert(foo.rfind("baz", 8) == 8, "");
  static_assert(foo.rfind("baz", 7) == SP::npos, "");
  static_assert(foo.rfind("x") == SP::npos, "");

  constexpr SP a = "a";

  static_assert(a.find("xxx") == SP::npos, "");
  static_assert(a.rfind("xxx") == SP::npos, "");
}

TEST(String, TrimWhitespace) {
  struct TestRow {
    std::string input;
    std::string ltrimmed;
    std::string rtrimmed;
    std::string trimmed;
  };
  std::vector<TestRow> testdata{
      {"foo", "foo", "foo", "foo"},     {"foo\t", "foo\t", "foo", "foo"},
      {"\tfoo", "foo", "\tfoo", "foo"}, {"\tfoo\t", "foo\t", "\tfoo", "foo"},
      {" \t\n\v\f\r", "", "", ""},
  };
  for (const auto& row : testdata) {
    SCOPED_TRACE(row.input);

    std::string tmp = row.input;
    base::ltrim_whitespace(&tmp);
    EXPECT_EQ(row.ltrimmed, tmp);

    tmp = row.input;
    base::rtrim_whitespace(&tmp);
    EXPECT_EQ(row.rtrimmed, tmp);

    tmp = row.input;
    base::trim_whitespace(&tmp);
    EXPECT_EQ(row.trimmed, tmp);
  }
}

TEST(String, TrimEOL) {
  struct TestRow {
    std::string input;
    std::string ltrimmed;
    std::string rtrimmed;
    std::string trimmed;
  };
  std::vector<TestRow> testdata{
      {"Line\r\n", "Line\r\n", "Line", "Line"},
      {"foo", "foo", "foo", "foo"},
      {"foo\n", "foo\n", "foo", "foo"},
      {"\nfoo", "foo", "\nfoo", "foo"},
      {"\nfoo\n", "foo\n", "\nfoo", "foo"},
      {"\n\r", "", "", ""},
  };
  for (const auto& row : testdata) {
    SCOPED_TRACE(row.input);

    std::string tmp = row.input;
    base::ltrim_whitespace(&tmp);
    EXPECT_EQ(row.ltrimmed, tmp);

    tmp = row.input;
    base::rtrim_whitespace(&tmp);
    EXPECT_EQ(row.rtrimmed, tmp);

    tmp = row.input;
    base::trim_whitespace(&tmp);
    EXPECT_EQ(row.trimmed, tmp);
  }
}

TEST(Splitter, Fixed) {
  auto splitter = base::split::fixed_length(3).limit(2);
  struct TestRow {
    SP input;
    std::vector<SP> expected;
  };
  std::vector<TestRow> testdata{
      {"abc", {"abc"}},
      {"abcd", {"abc", "d"}},
      {"abcde", {"abc", "de"}},
      {"abcdef", {"abc", "def"}},
      {"abcdefg", {"abc", "defg"}},
  };
  for (const auto& row : testdata) {
    SCOPED_TRACE(row.input);
    EXPECT_PRED_FORMAT2(vec_eq, row.expected, splitter.split(row.input));
  }
}

TEST(Splitter, Char) {
  auto splitter = base::split::on(',').limit(2);
  struct TestRow {
    SP input;
    std::vector<SP> expected;
  };
  std::vector<TestRow> testdata{
      {"a", {"a"}}, {"a,b", {"a", "b"}}, {"a,b,c", {"a", "b,c"}},
  };
  for (const auto& row : testdata) {
    SCOPED_TRACE(row.input);
    EXPECT_PRED_FORMAT2(vec_eq, row.expected, splitter.split(row.input));
  }

  splitter.unlimited();
  testdata = std::vector<TestRow>{
      {"a", {"a"}},
      {"a,b", {"a", "b"}},
      {"a,b,c", {"a", "b", "c"}},
      {",a,b,c", {"", "a", "b", "c"}},
      {"a,,b,c", {"a", "", "b", "c"}},
      {"a,b,,c", {"a", "b", "", "c"}},
      {"a,b,c,", {"a", "b", "c", ""}},
      {" a , b ", {" a ", " b "}},
      {" a , b , , c ", {" a ", " b ", " ", " c "}},
  };
  for (const auto& row : testdata) {
    SCOPED_TRACE(row.input);
    EXPECT_PRED_FORMAT2(vec_eq, row.expected, splitter.split(row.input));
  }

  splitter.trim_whitespace().omit_empty();
  testdata = std::vector<TestRow>{
      {"a", {"a"}},
      {"a,b", {"a", "b"}},
      {"a,b,c", {"a", "b", "c"}},
      {",a,b,c", {"a", "b", "c"}},
      {"a,,b,c", {"a", "b", "c"}},
      {"a,b,,c", {"a", "b", "c"}},
      {"a,b,c,", {"a", "b", "c"}},
      {" a , b ", {"a", "b"}},
      {" a , b , , c ", {"a", "b", "c"}},
  };
  for (const auto& row : testdata) {
    SCOPED_TRACE(row.input);
    EXPECT_PRED_FORMAT2(vec_eq, row.expected, splitter.split(row.input));
  }
}

TEST(Splitter, Str) {
  auto splitter = base::split::on("<>").limit(2);
  struct TestRow {
    SP input;
    std::vector<SP> expected;
  };
  std::vector<TestRow> testdata{
      {"a", {"a"}}, {"a<>b", {"a", "b"}}, {"a<>b<>c", {"a", "b<>c"}},
  };
  for (const auto& row : testdata) {
    SCOPED_TRACE(row.input);
    EXPECT_PRED_FORMAT2(vec_eq, row.expected, splitter.split(row.input));
  }
}

TEST(Splitter, Pred) {
  auto ws = [](char ch) { return ch == ' ' || ch == '\t'; };
  auto splitter = base::split::on(ws).limit(2);
  struct TestRow {
    SP input;
    std::vector<SP> expected;
  };
  std::vector<TestRow> testdata{
      {"a", {"a"}},

      {"a b", {"a", "b"}},
      {"a b c", {"a", "b c"}},
      {"a b\tc", {"a", "b\tc"}},

      {"a\tb", {"a", "b"}},
      {"a\tb c", {"a", "b c"}},
      {"a\tb\tc", {"a", "b\tc"}},
  };
  for (const auto& row : testdata) {
    SCOPED_TRACE(row.input);
    EXPECT_PRED_FORMAT2(vec_eq, row.expected, splitter.split(row.input));
  }
}

TEST(Splitter, Pattern) {
  auto splitter = base::split::on_pattern("<-*>").limit(2);
  struct TestRow {
    SP input;
    std::vector<SP> expected;
  };
  std::vector<TestRow> testdata{
      {"a", {"a"}},

      {"a<>b", {"a", "b"}},
      {"a<->b", {"a", "b"}},

      {"a<>b<>c", {"a", "b<>c"}},
      {"a<->b<->c", {"a", "b<->c"}},
  };
  for (const auto& row : testdata) {
    SCOPED_TRACE(row.input);
    EXPECT_PRED_FORMAT2(vec_eq, row.expected, splitter.split(row.input));
  }
}

TEST(Joiner, Empty) {
  struct TestRow {
    std::vector<SP> input;
    std::string expected;
  };

  auto joiner = base::join::on();
  std::vector<TestRow> testdata{
      {{}, ""}, {{"a"}, "a"}, {{"a", "b"}, "ab"}, {{"a", "b", "c"}, "abc"},
  };
  for (const auto& row : testdata) {
    SCOPED_TRACE(S(row.input));
    EXPECT_EQ(row.expected, S(joiner.join(row.input)));
  }
}

TEST(Joiner, Char) {
  struct TestRow {
    std::vector<SP> input;
    std::string expected;
  };

  auto joiner = base::join::on(',');
  std::vector<TestRow> testdata{
      {{}, ""},
      {{"a"}, "a"},
      {{"a", "b"}, "a,b"},
      {{"a", "b", "c"}, "a,b,c"},
      {{"", "a", "b", "c"}, ",a,b,c"},
      {{"a", "", "b", "c"}, "a,,b,c"},
      {{"a", "b", "", "c"}, "a,b,,c"},
      {{"a", "b", "c", ""}, "a,b,c,"},
  };
  for (const auto& row : testdata) {
    SCOPED_TRACE(S(row.input));
    EXPECT_EQ(row.expected, S(joiner.join(row.input)));
  }

  joiner.skip_empty();
  testdata = std::vector<TestRow>{
      {{}, ""},
      {{"a"}, "a"},
      {{"a", "b"}, "a,b"},
      {{"a", "b", "c"}, "a,b,c"},
      {{"", "a", "b", "c"}, "a,b,c"},
      {{"a", "", "b", "c"}, "a,b,c"},
      {{"a", "b", "", "c"}, "a,b,c"},
      {{"a", "b", "c", ""}, "a,b,c"},
  };
  for (const auto& row : testdata) {
    SCOPED_TRACE(S(row.input));
    EXPECT_EQ(row.expected, S(joiner.join(row.input)));
  }
}

TEST(Joiner, String) {
  struct TestRow {
    std::vector<SP> input;
    std::string expected;
  };

  auto joiner = base::join::on("<>");
  std::vector<TestRow> testdata{
      {{}, ""},
      {{"a"}, "a"},
      {{"a", "b"}, "a<>b"},
      {{"a", "b", "c"}, "a<>b<>c"},
  };
  for (const auto& row : testdata) {
    SCOPED_TRACE(S(row.input));
    EXPECT_EQ(row.expected, S(joiner.join(row.input)));
  }
}
