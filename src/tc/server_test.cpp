// Copyright (c) Maia

#include "tc/server.h"

#include <gtest/gtest.h>

namespace tc {

namespace {

class MockFileFinder : public IFileFinder {
 public:
  std::vector<std::string> GetFiles() override {
    return {"path/to/mock_file1.cpp", "path/to/mock_file2.h"};
  }
};

template <typename Fn>
[[nodiscard]] auto CaptureStdOut(Fn&& callable) -> std::string {
  std::stringstream captured_output;
  auto* original_buf = std::cout.rdbuf();
  std::cout.rdbuf(captured_output.rdbuf());

  callable();

  std::cout.rdbuf(original_buf);
  return captured_output.str();
}

}  // namespace

TEST(ReadMessageTest, HandlesValidMessage) {
  std::string raw_message =
      "Content-Length: 27\r\n\r\n{\"key\":\"value\",\"id\":42}";
  std::stringstream input(raw_message);

  auto msg = ReadMessage(input);

  ASSERT_TRUE(msg);
  EXPECT_EQ((*msg)["key"], "value");
  EXPECT_EQ((*msg)["id"], 42);
}

TEST(ReadMessageTest, DoesNotHandlesMessageWithExtraHeaders) {
  std::string raw_message =
      "Content-Type: application/vscode-jsonrpc; "
      "charset=utf-8\r\nContent-Length: 18\r\n\r\n{\"data\":\"test\"}";
  std::stringstream input(raw_message);

  auto msg = ReadMessage(input);

  ASSERT_FALSE(msg.has_value());
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

  auto captured_output = CaptureStdOut([&] { HandleInitialize(request); });
  std::stringstream ss;
  ss << captured_output;

  // Parse the captured output and verify it's a valid response
  auto response = ReadMessage(ss);
  ASSERT_TRUE(response.has_value());
  EXPECT_EQ((*response)["id"], request["id"]);
  EXPECT_TRUE(response->contains("result"));
}

TEST(RpcUnitTest, HandleQueryFilesStreamsNotifications) {
  nlohmann::json request = {
      {"jsonrpc",                     "2.0"},
      {     "id",                         2},
      { "method",              "queryFiles"},
      { "params", {{"query", "test_query"}}}
  };

  MockFileFinder mock_finder;

  // Pass the mock finder to the function.
  std::string captured_output =
      CaptureStdOut([&]() { HandleQueryFiles(request, mock_finder); });

  std::stringstream ss;
  ss << captured_output;

  // The mock implementation sends 2 results. We will read them back.
  for (int i = 0; i < 2; ++i) {
    auto notification = ReadMessage(ss);
    ASSERT_TRUE(notification.has_value()) << ss.str();

    EXPECT_EQ((*notification)["method"], "$/query/fileResults");
    EXPECT_FALSE(notification->contains("id"));
    EXPECT_TRUE((*notification)["params"].contains("file"));
  }
}

}  // namespace tc
