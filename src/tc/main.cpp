// Copyright (c) Maia

#include <iostream>
#include <print>

#include <boost/asio.hpp>
#include <boost/process.hpp>

#include "tc/server.h"

namespace tt {

namespace {

bool IsValidRequest(const nlohmann::json& request) {
  return request.contains("jsonrpc") && request["jsonrpc"] == "2.0" &&
         request.contains("method");
}

int Run() {
  boost::asio::io_context ioc;

  FdFileFinder file_finder{};

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
      HandleQueryFiles(request, file_finder);
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

}  // namespace

}  // namespace tt

int main() {
  try {
    tt::Run();
  } catch (std::exception& e) {
    std::print(stderr, "Error: {}\n", e.what());
  }
  return 0;
}
