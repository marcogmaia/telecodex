// Copyright (c) Maia

#include <Windows.h>

#include <iostream>

#include <lsp/connection.h>
#include <lsp/io/standardio.h>
#include <lsp/messagehandler.h>
#include <lsp/messages.h>
#include <lsp/process.h>
#include <boost/asio.hpp>

namespace lsp {

struct QueryFiles {
  static constexpr auto Method = std::string_view("queryFiles");
  static constexpr auto Direction = lsp::MessageDirection::ClientToServer;
  static constexpr auto Type = lsp::Message::Request;

  // using ErrorData = lsp::InitializeError;
  // using Params = bool;
  using Result = std::string;
};

}

int main() {
  auto connection = lsp::Connection(lsp::io::standardIO());
  auto message_handler = lsp::MessageHandler(connection);

  message_handler.add<lsp::requests::Initialize>(
      [](const lsp::requests::Initialize::Params& /*params*/) {
        auto capabilities = lsp::ServerCapabilities{
            .positionEncoding = lsp::PositionEncodingKind::UTF16};

        auto server_info = lsp::InitializeResultServerInfo{
            .name = "Language Server", .version = "1.0.0"};

        return lsp::requests::Initialize::Result{
            .capabilities = capabilities,
            .serverInfo = server_info,
        };
      });

  bool running = true;

  message_handler.add<lsp::QueryFiles>([&running]() {
    // std::cout << "message received\n";
    // running = false;
    return std::string("oie");
  });

  while (running) {
    message_handler.processIncomingMessages();
    break;
  }

  return 0;
}
