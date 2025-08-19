// Copyright (c) Maia

#include "tc/server.h"

#include <gtest/gtest.h>

namespace tc {

TEST(ReadMessageTest, HandlesValidMessage) {
  std::string raw_message =
      "Content-Length: 27\r\n\r\n{\"key\":\"value\",\"id\":42}";
  std::stringstream input(raw_message);

  auto msg = ReadMessage(input);

  ASSERT_TRUE(msg);
  EXPECT_EQ((*msg)["key"], "value");
  EXPECT_EQ((*msg)["id"], 42);
}

TEST(ReadMessageTest, HandlesMessageWithExtraHeaders) {
  std::string raw_message =
      "Content-Type: application/vscode-jsonrpc; "
      "charset=utf-8\r\nContent-Length: 18\r\n\r\n{\"data\":\"test\"}";
  std::stringstream input(raw_message);

  auto msg = ReadMessage(input);

  ASSERT_TRUE(msg.has_value());
  EXPECT_EQ((*msg)["data"], "test");
}

TEST(ReadMessageTest, FailsOnInvalidContentLength) {
  std::string raw_message = "Content-Length: not-a-number\r\n\r\n{}";
  std::stringstream input(raw_message);

  auto msg = ReadMessage(input);

  ASSERT_FALSE(msg.has_value());
}

TEST(ReadMessageTest, FailsOnMalformedJson) {
  std::string raw_message = "Content-Length: 15\r\n\r\n{\"key\":invalid}";
  std::stringstream input(raw_message);

  auto msg = ReadMessage(input);

  ASSERT_FALSE(msg.has_value());
}

TEST(ReadMessageTest, FailsOnNoContentLength) {
  std::string raw_message = "Some-Other-Header: value\r\n\r\n{}";
  std::stringstream input(raw_message);

  auto msg = ReadMessage(input);

  ASSERT_FALSE(msg.has_value());
}

TEST(HandleInitializeTest, RespondsCorrectly) {
  nlohmann::json request = {
      {"jsonrpc",                             "2.0"},
      {     "id",                                 1},
      { "method",                      "initialize"},
      { "params", {{"workspaceRoot", "/test/path"}}}
  };

  // Redirect cout to capture the output
  std::stringstream captured_output;
  auto* original_cout_buf = std::cout.rdbuf();
  std::cout.rdbuf(captured_output.rdbuf());

  HandleInitialize(request);

  // Restore cout
  std::cout.rdbuf(original_cout_buf);

  // Parse the captured output and verify it's a valid response
  auto response = ReadMessage(captured_output);
  ASSERT_TRUE(response.has_value());
  EXPECT_EQ((*response)["id"], request["id"]);
  EXPECT_TRUE(response->contains("result"));
}

}  // namespace tc
