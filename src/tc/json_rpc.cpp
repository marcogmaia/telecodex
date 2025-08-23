// Copyright (c) Maia

#include "tc/json_rpc.h"

#include <charconv>
#include <iostream>
#include <thread>

#include <boost/asio.hpp>

namespace tc {

namespace detail {

std::string GetHeader(std::istream& istream) {
  std::string buffer(64, 0);
  istream.get(buffer.data(), buffer.size(), '\r');
  buffer.resize(istream.gcount());
  return buffer;
}

// Left trim: remove leading whitespace
void ltrim(std::string& s) {
  const auto it = std::find_if(s.begin(), s.end(), [](char c) {
    return !std::isspace(c, std::locale{});  // or use ASCII-only check
  });
  s.erase(s.begin(), it);
}

// Right trim: remove trailing whitespace
void rtrim(std::string& s) {
  const auto it = std::find_if(s.rbegin(), s.rend(), [](char c) {
    return !std::isspace(c, std::locale{});
  });
  s.erase(it.base(), s.end());
}

void trim(std::string& s) {
  ltrim(s);
  rtrim(s);
}

// NOTE: This function trims all the leading whitespaces and trailing spaces.
// And when a line is read, it also leaves no white spaces in the stream until
// the first non space character.
std::optional<std::string> ReadNextContentLine(std::istream& input) {
  std::string line;

  // Guard: invalid stream state
  if (!input.good()) {
    return std::nullopt;
  }

  // Read next non-blank line (skipping whitespace).
  if (!std::getline(std::ws(input), line)) {
    // If we hit EOF and have a partial line, keep it.
    if (!input.eof() || line.empty()) {
      return std::nullopt;
    }
    // Otherwise, we have a partial line at EOF — proceed to trim.
  }

  // Remove remaining white spaces (of the header), next step would be to read
  // the json content.
  std::ws(input);

  trim(line);

  // Reject lines that are empty after trimming.
  if (line.empty()) {
    return std::nullopt;
  }

  return line;
}

std::optional<std::string> ReadJsonString(std::istream& input, int length) {
  std::string result;
  result.resize(length);

  // Read exactly 'length' characters from the stream
  input.read(result.data(), length);

  // Verify we read exactly what we expected
  if (input.gcount() != length) {
    return std::nullopt;
  }

  return result;
}

std::optional<int> ParseContentLength(std::string_view content) {
  // Expected format: "Content-Length: <number>"
  constexpr std::string_view kPrefix = "Content-Length: ";

  // Check if the content starts with the expected prefix
  if (content.size() < kPrefix.size() || !content.starts_with(kPrefix)) {
    return std::nullopt;
  }

  // Extract the number part (skip the prefix)
  std::string_view number_sv = content.substr(kPrefix.size());
  std::string number_str{number_sv};

  // Parse the integer
  int value = 0;
  auto result = std::from_chars(
      number_str.data(), number_str.data() + number_str.size(), value);

  // Check if parsing was successful and consumed the entire number string
  bool has_error = result.ec != std::errc{};  // NOLINT
  if (has_error || result.ptr != number_str.data() + number_str.size()) {
    return std::nullopt;
  }

  return value;
}

}  // namespace detail

struct JsonRpc::Impl {
  // boost::asio::io_context ioc;
  // AsyncConsoleReader cr{ioc};
  std::string buffer;
  std::jthread task;
};

JsonRpc::JsonRpc()
    : impl_(std::make_unique<Impl>()) {}

JsonRpc::~JsonRpc() = default;

void JsonRpc::Task() {
  impl_->buffer.clear();
  auto buffer = boost::asio::dynamic_buffer(impl_->buffer);
}

void JsonRpc::Init() {}

}  // namespace tc
