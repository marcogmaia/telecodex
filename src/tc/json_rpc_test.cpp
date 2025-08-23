// Copyright (c) Maia

#include "tc/json_rpc.h"

#include <format>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

namespace tc {

// TEST(JsonRpc, SimpleTest) {
//   JsonRpc rpc{};
//   rpc.Init();

//   // // int a;
//   // // std::cin >> a;
//   // std::println("{} wat", a);

//   std::this_thread::sleep_for(std::chrono::milliseconds(5000));
// }

namespace detail {

std::optional<std::string> ReadNextContentLine(std::istream& input);
std::optional<int> ParseContentLength(std::string_view content);
std::optional<std::string> ReadJsonString(std::istream& input, int length);

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

TEST(ReadNextContentLine, ContentHeader) {
  std::istringstream input("Content-Length: 7\r\n\r\nContent");
  auto res1 = ReadNextContentLine(input);
  auto res2 = ReadNextContentLine(input);
  ASSERT_TRUE(res1);
  EXPECT_EQ(*res1, "Content-Length: 7");
  ASSERT_TRUE(res2);
  EXPECT_EQ(*res2, "Content");
}

TEST(ReadJsonMessage, ReadUnformattedJson) {
  std::string json_part = "{\"number\": \n42}\n";
  auto input_str =
      std::format("Content-Length: {}\r\n\r\n{}", json_part.size(), json_part);
  std::istringstream input(input_str);
  auto content = ReadNextContentLine(input);
  // auto num = scn::scan<int>(*content, "Content-Length: {}");
  auto num = ParseContentLength(*content);
  ASSERT_TRUE(num);
  EXPECT_EQ(num.value(), 16);

  auto json_result = ReadJsonString(input, num.value());
  ASSERT_TRUE(json_result);
  EXPECT_EQ(*json_result, json_part);
}

}  // namespace detail

}  // namespace tc
