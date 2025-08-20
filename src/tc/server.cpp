/// Copyright (c) Maia

#include "tc/server.h"

#include <iostream>
#include <print>
#include <vector>

#include <scn/scan.h>

#include "tc/file_finder.h"

namespace tc {

void WriteMessage(const nlohmann::json& message) {
  const std::string content = message.dump();
  std::print(
      std::cout, "Content-Length: {}\r\n\r\n{}", content.size(), content);
}

std::optional<int> ParseContentLengthWithBuffer(std::istream& input) {
  std::vector<char> buffer(128);  // Start with a reasonable buffer size
  input.read(buffer.data(), buffer.size());

  std::string_view header_view(buffer.data(), input.gcount());

  // Find the end of the headers.
  auto end_of_headers = header_view.find("\r\n\r\n");
  if (end_of_headers == std::string_view::npos) {
    return std::nullopt;
  }

  // Find the "Content-Length: " string.
  auto cl_pos = header_view.find("Content-Length: ");
  if (cl_pos == std::string_view::npos) {
    return std::nullopt;
  }

  // Isolate the number part.
  auto value_start = cl_pos + 16;
  auto value_end = header_view.find("\r\n", value_start);
  if (value_end == std::string_view::npos) {
    return std::nullopt;
  }

  std::string_view length_sv =
      header_view.substr(value_start, value_end - value_start);

  // Safely parse the number from the string_view
  int content_length;
  auto [ptr, ec] = std::from_chars(
      length_sv.data(), length_sv.data() + length_sv.size(), content_length);

  if (ec != std::errc()) {  // NOLINT
    return std::nullopt;
  }

  // Crucially, we must "put back" the unread body part into the stream
  // so the next read operation can consume it. The clear() is here because we
  // might read everything in `input`, thus triggering the `eof` bit.
  input.clear();
  input.seekg(end_of_headers + 4 - input.gcount(), std::ios_base::cur);

  return content_length;
}

void WriteErrorResponse(const std::optional<nlohmann::json>& id,
                        int code,
                        const std::string& message) {
  nlohmann::json error_response = {
      {"jsonrpc",                                  "2.0"},
      {     "id",         id.has_value() ? *id : nullptr},
      {  "error", {{"code", code}, {"message", message}}}
  };
  WriteMessage(error_response);
}

std::optional<nlohmann::json> ReadMessage(std::istream& input) {
  // I feel like the right way to do this is with asio.
  std::vector<char> buffer(64);
  input.getline(buffer.data(), buffer.size(), '\r');
  // Ignore the remaining "\n\r\n".
  input.ignore(3);
  auto content_str = std::string_view(buffer.data(), input.tellg());
  auto res = scn::scan<int>(content_str, "Content-Length: {}");
  if (!res) {
    return std::nullopt;
  }

  int content_length = res->value();

  if (!content_length) {
    input.ignore(std::numeric_limits<int>::max());
    return std::nullopt;
  }

  // Read the exact number of bytes for the JSON content.
  std::vector<char> content(content_length);
  input.read(content.data(), content_length);

  try {
    auto json = nlohmann::json::parse(content);
    std::print(stderr, "Received:\n{}\n", json.dump());
    return json;
  } catch (const nlohmann::json::parse_error& e) {
    // Let the main loop handle sending the Parse Error response.
    // TODO: At least log something.
    input.ignore(std::numeric_limits<int>::max());
    return std::nullopt;
  }
}

void HandleInitialize(const nlohmann::json& request) {
  // TODO: Start indexing files here.
  // TODO: Check a better way to log, without using stderr, this is a hacky way
  // to get something running initially.
  std::cerr << "Initialize request received.\n" << request.dump(2);

  // Send a response back to the client.
  nlohmann::json response = {
      {"jsonrpc",                  "2.0"},
      {     "id",          request["id"]},
      // Announce server capabilities here
      { "result", {{"capabilities", {}}}}
  };
  WriteMessage(response);
}

void HandleQueryFiles(const nlohmann::json& request, IFileFinder& file_finder) {
  std::string query = request["params"]["query"];
  std::cerr << "Received query: " << query << '\n';

  // TODO: Move this to proper place where the indexing should occur. We would
  // nee to use a system watcher to keep the index always up to date.
  auto files = file_finder.GetFiles();

  for (const auto& file : files) {
    // Create and send a notification for each result.
    // Notifications do not have an "id" field.
    nlohmann::json notification = {
        {"jsonrpc",                 "2.0"},
        { "method", "$/query/fileResults"},
        { "params",      {{"file", file}}}
    };
    WriteMessage(notification);
  }
}

}  // namespace tc
