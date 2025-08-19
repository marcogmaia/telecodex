/// Copyright (c) Maia

#include "tc/server.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

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

  // Simulate finding files and stream a notification for each one.
  // This demonstrates the streaming results requirement.
  std::vector<std::string> dummy_files = {
      "src/main.cpp", "src/utils.hpp", "readme.md"};

  for (const auto& file : dummy_files) {
    // Create and send a notification for each result.
    // Notifications do not have an "id" field.
    nlohmann::json notification = {
        {"jsonrpc",                 "2.0"},
        { "method", "$/query/fileResults"},
        { "params",      {{"file", file}}}
    };
    WriteMessage(notification);
    // Add a small delay to simulate a real search.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }
}

namespace {

bool IsValidRequest(const nlohmann::json& request) {
  return request.contains("jsonrpc") && request["jsonrpc"] == "2.0" &&
         request.contains("method");
}

}  // namespace

int Run() {
  while (true) {
    auto message = ReadMessage(std::cin);
    if (!message) {
      if (std::cin.eof()) {
        break;
      }
      // TODO: Use an enum instead of these magic numbers to enum.
      // Per spec, a Parse Error is sent if the JSON is invalid.
      WriteErrorResponse(nullptr, -32700, "Parse error");
      continue;
    }

    const nlohmann::json& request = *message;
    auto id =
        request.contains("id") ? std::optional(request["id"]) : std::nullopt;

    if (!IsValidRequest(request)) {
      WriteErrorResponse(id, -32600, "Invalid Request");
    }

    // Notifications do not have an ID and do not get responses.
    // We only need to respond to requests that have an ID.
    std::string method = request["method"];

    // Dispatch to the correct handler based on the method.
    if (method == "initialize") {
      HandleInitialize(request);
    } else if (method == "queryFiles") {
      HandleQueryFiles(request);
    } else if (method == "exit") {
      break;
    } else {
      // Per spec, send an error if the method is unknown.
      if (id.has_value()) {
        WriteErrorResponse(id, -32601, "Method not found");
      }
    }
  }

  return 0;
}

}  // namespace tc
