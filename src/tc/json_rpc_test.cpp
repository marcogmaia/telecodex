// Copyright (c) Maia

#include "tc/json_rpc.h"

#include <print>

#include <gtest/gtest.h>

namespace tc {

namespace detail {

std::string GetHeader(std::istream& istream);

}  // namespace detail

TEST(JsonRpc, GetHeader) {
  std::stringstream ss;
  ss << "Content-Length: 42\r\n\r\n";
  auto str = detail::GetHeader(ss);
  EXPECT_EQ(str, "Content-Length: 42");
}

// TEST(JsonRpc, SimpleTest) {
//   JsonRpc rpc{};
//   rpc.Init();

//   // // int a;
//   // // std::cin >> a;
//   // std::println("{} wat", a);

//   std::this_thread::sleep_for(std::chrono::milliseconds(5000));
// }

namespace detail {

// Forward declaration of the function under test
std::optional<std::string> ReadNextContentLine(std::istream& input);

// Test: Successfully read a line with newline
TEST(ReadNextContentLine, ReadLineWithNewline) {
  std::istringstream input("Hello, World!\nMore text");
  auto result = ReadNextContentLine(input);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, "Hello, World!");
}

// Test: Read multiple lines
TEST(ReadNextContentLine, ReadMultipleLines) {
  std::istringstream input("First\nSecond\nThird\n");

  auto line1 = ReadNextContentLine(input);
  ASSERT_TRUE(line1.has_value());
  EXPECT_EQ(*line1, "First");

  auto line2 = ReadNextContentLine(input);
  ASSERT_TRUE(line2.has_value());
  EXPECT_EQ(*line2, "Second");

  auto line3 = ReadNextContentLine(input);
  ASSERT_TRUE(line3.has_value());
  EXPECT_EQ(*line3, "Third");
}

// Test: Handle partial line at EOF (no trailing newline)
TEST(ReadNextContentLine, ReadPartialLineAtEOF) {
  std::istringstream input("Incomplete line without newline");
  auto result = ReadNextContentLine(input);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, "Incomplete line without newline");
}

// Test: Empty stream (immediate EOF)
TEST(ReadNextContentLine, EmptyStream) {
  std::istringstream input("");
  auto result = ReadNextContentLine(input);

  EXPECT_FALSE(result.has_value());  // Should return nullopt
}

// Test: Stream with only whitespace and no newline
TEST(ReadNextContentLine, WhitespaceOnlyNoNewline) {
  std::istringstream input("   \t  ");
  auto result = ReadNextContentLine(input);

  EXPECT_FALSE(result);
}

// Test: Empty line (just a newline)
TEST(ReadNextContentLine, EmptyLine) {
  std::istringstream input("\nNot empty");
  auto result = ReadNextContentLine(input);

  ASSERT_TRUE(result.has_value());
  // EXPECT_FALSE(result) << *result;
  // auto result2 = ReadLine(input);
  // ASSERT_TRUE(result);
  EXPECT_EQ(*result, "Not empty");
}

// Test: Multiple consecutive newlines
TEST(ReadNextContentLine, ConsecutiveNewlines) {
  std::istringstream input("\n\n\n");
  auto line1 = ReadNextContentLine(input);
  auto line2 = ReadNextContentLine(input);
  auto line3 = ReadNextContentLine(input);
  auto line4 = ReadNextContentLine(input);
  EXPECT_FALSE(line1);
  EXPECT_FALSE(line2);
  EXPECT_FALSE(line3);
  EXPECT_FALSE(line4);
}

// Test: Mixed content with partial EOF
TEST(ReadNextContentLine, MixedContentWithPartialEOF) {
  std::istringstream input("Line 1\nLine 2\nFinal partial");

  auto line1 = ReadNextContentLine(input);
  ASSERT_TRUE(line1.has_value());
  EXPECT_EQ(*line1, "Line 1");

  auto line2 = ReadNextContentLine(input);
  ASSERT_TRUE(line2.has_value());
  EXPECT_EQ(*line2, "Line 2");

  auto line3 = ReadNextContentLine(input);
  ASSERT_TRUE(line3.has_value());
  EXPECT_EQ(*line3, "Final partial");
}

// Test: Stream in failed state
TEST(ReadNextContentLine, FailedStream) {
  std::istringstream input("test");
  input.setstate(std::ios::failbit);  // Manually set failbit

  auto result = ReadNextContentLine(input);
  EXPECT_FALSE(result.has_value());
}

// Test: Stream with leading and trailing spaces and newline
TEST(ReadNextContentLine, LineWithSpaces) {
  std::istringstream input("  leading and trailing  \nnext line");
  auto result = ReadNextContentLine(input);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, "leading and trailing");
}

// Test: Very long line
TEST(ReadNextContentLine, VeryLongLine) {
  std::string long_content(1000, 'x');  // 1000 'x' characters
  long_content += "\n";
  std::istringstream input(long_content);

  auto result = ReadNextContentLine(input);
  ASSERT_TRUE(result.has_value());
  long_content.pop_back();
  EXPECT_EQ(*result, long_content);
}

// Test: Unicode/text with special characters
TEST(ReadNextContentLine, SpecialCharacters) {
  constexpr const char* kText =
      "Hello © 2024 你好\n"
      "Next line\n";
  std::istringstream input(kText);

  auto result1 = ReadNextContentLine(input);
  auto result2 = ReadNextContentLine(input);

  ASSERT_TRUE(result1.has_value());
  EXPECT_EQ(*result1, "Hello © 2024 你好");
  ASSERT_TRUE(result2.has_value());
  EXPECT_EQ(*result2, "Next line");
}

}  // namespace detail

}  // namespace tc
