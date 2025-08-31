// Copyright (c) Maia

#include <Windows.h>

#include <fstream>
#include <ranges>

#include <lsp/connection.h>
#include <lsp/io/standardio.h>
#include <lsp/messagehandler.h>
#include <lsp/process.h>
#include <boost/asio.hpp>

#include "tt/messages.h"

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

  auto of = std::ofstream(
      "C:/Users/marco/Documents/dev/projects/teletex/teletex/log.log");

  message_handler.add<lsp::requests::QueryFiles>(
      [&](lsp::QueryFilesParams uri) {
        auto root_dir = std::string(uri.root.path());
        of << "[log] " << root_dir << '\n';
        return lsp::QueryFilesResult(ToUri(ff.GetFilesFromDir(root_dir)));
      });

  message_handler.add<lsp::notifications::Exit>([&] {
    of << "[log] " << "exiting...";
    running = false;
  });

  while (running) {
    message_handler.processIncomingMessages();
  }

  return 0;
}
