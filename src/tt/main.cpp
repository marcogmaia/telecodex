// Copyright (c) Maia

#include <Windows.h>

#include <ranges>

#include <lsp/connection.h>
#include <lsp/io/standardio.h>
#include <lsp/messagehandler.h>
#include <lsp/messages.h>
#include <lsp/process.h>
#include <boost/asio.hpp>

#include "tt/file_finder.h"

namespace {

std::vector<lsp::DocumentUri> ToUri(
    const std::vector<std::string>& file_paths) {
  return file_paths | std::views::transform(lsp::DocumentUri::fromPath) |
         std::ranges::to<std::vector<lsp::DocumentUri>>();
}

}  // namespace

int main() {
  auto connection = lsp::Connection(lsp::io::standardIO());
  auto message_handler = lsp::MessageHandler(connection);

  bool running = true;

  tt::FdFileFinder ff;

  message_handler.add<lsp::requests::QueryFiles>(
      [&]() { return lsp::QueryFilesResult(ToUri(ff.GetFiles())); });

  message_handler.add<lsp::notifications::Exit>(
      [&running] { running = false; });

  while (running) {
    message_handler.processIncomingMessages();
  }

  return 0;
}
