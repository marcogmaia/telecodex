// Copyright (c) Maia

#include "tt/file_finder.h"

#include <print>
#include <ranges>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/process.hpp>

namespace tt {

std::vector<std::string> FdFileFinder::GetFilesFromDir(std::string root_dir) {
  namespace asio = boost::asio;
  namespace bp = boost::process;

  io_context_.restart();

  asio::readable_pipe readable_pipe{io_context_};

  auto fd_path = bp::environment::find_executable("fd.exe");

  boost::process::process proc(
      io_context_.get_executor(),
      fd_path,
      {"--type", "f", ".", root_dir},
      boost::process::process_stdio{.in = {}, .out = readable_pipe, .err = {}});

  asio::streambuf buffer;
  std::string sbuffer;

  asio::async_read(
      readable_pipe,
      buffer,
      [&](const boost::system::error_code& ec, std::size_t bytes_transferred) {
        if (!ec || ec == asio::error::eof || ec == asio::error::broken_pipe) {
          const auto data = buffer.data();
          sbuffer = std::string(asio::buffers_begin(data),
                                asio::buffers_begin(data) + bytes_transferred);
        } else {
          std::print(stderr, "Read Error: {}\n", ec.message());
        }
      });

  proc.async_wait([&](std::error_code, int exit_code) {});

  io_context_.run();

  auto files = sbuffer | std::views::drop_while(isspace) | std::views::reverse |
               std::views::drop_while(isspace) | std::views::reverse |
               std::views::split('\n') |
               std::ranges::to<std::vector<std::string>>();

  return files;
}

std::vector<std::string> FdFileFinder::GetFiles() {
  return GetFilesFromDir(".");
}

}  // namespace tt
