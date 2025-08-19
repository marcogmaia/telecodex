/// Copyright (c) Maia

#include "tc/server.h"

#include <iostream>
#include <vector>
#include "tc/file_finder.h"

#include <scn/scan.h>

namespace tc {

void WriteMessage(const nlohmann::json& message) {
  const std::string content = message.dump();
  // The Language Server Protocol requires this specific header format.
  std::cout << "Content-Length: " << content.length() << "\r\n\r\n"
            << content << std::flush;
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
  std::string line;
  int content_length = -1;

  // First, read headers until we find "Content-Length" and the separating blank
  // line.
  while (std::getline(input, line) && !line.empty() && line != "\r") {
    // Use scn::scan to find and parse the Content-Length header.
    auto result = scn::scan<int>(line, "Content-Length: {}");
    if (result) {
      content_length = result->value();
    }
    // We continue reading to consume any other headers.
  }

  if (content_length <= 0) {
    return std::nullopt;
  }

  // Read the exact number of bytes for the JSON content.
  std::vector<char> content(content_length);
  input.read(content.data(), content_length);

  try {
    return nlohmann::json::parse(content);
  } catch (const nlohmann::json::parse_error& e) {
    // Let the main loop handle sending the Parse Error response.
    return std::nullopt;
  }
}

void HandleInitialize(const nlohmann::json& request) {
  // TODO: start indexing files here.

  // Send a response back to the client.
  nlohmann::json response = {
      {"jsonrpc",                  "2.0"},
      {     "id",          request["id"]},
      // Announce server capabilities here
      { "result", {{"capabilities", {}}}}
  };
  WriteMessage(response);
}

void HandleQueryFiles(const nlohmann::json& request) {
  std::string query = request["params"]["query"];
  std::cerr << "Received query: " << query << '\n';

  // TODO: Move this to proper place where the indexing should occur. We would
  // nee to use a system watcher to keep the index always up to date.
  FdFileFinder finder{};
  auto files = finder.GetFiles();

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
