// Copyright (c) Maia

#include <print>
#include <ranges>

#include <boost/asio.hpp>
#include <boost/process.hpp>

class IFileFinder {
 public:
  virtual ~IFileFinder() = default;

  virtual std::vector<std::string> GetFiles() = 0;
};

class FdFileFinder : public IFileFinder {
 public:
  std::vector<std::string> GetFiles() override {
    namespace asio = boost::asio;
    namespace bp = boost::process;

    asio::readable_pipe readable_pipe{io_context_};

    auto fd_path = bp::environment::find_executable("fd.exe");

    boost::process::process proc(
        io_context_.get_executor(),
        fd_path,
        {"--type", "file"},
        boost::process::process_stdio{
            .in = {}, .out = readable_pipe, .err = {}});

    asio::streambuf buffer;
    std::string sbuffer;

    asio::async_read(
        readable_pipe,
        buffer,
        [&](const boost::system::error_code& ec,
            std::size_t bytes_transferred) {
          if (!ec || ec == asio::error::eof || ec == asio::error::broken_pipe) {
            const auto data = buffer.data();
            sbuffer =
                std::string(asio::buffers_begin(data),
                            asio::buffers_begin(data) + bytes_transferred);
          } else {
            std::print("Read Error: {}\n", ec.message());
          }
        });

    proc.async_wait([&](std::error_code, int exit_code) {});

    io_context_.run();

    auto files = sbuffer | std::views::drop_while(isspace) |
                 std::views::reverse | std::views::drop_while(isspace) |
                 std::ranges::views::reverse | std::views::split('\n') |
                 std::ranges::to<std::vector<std::string>>();

    return files;
  }

 private:
  boost::asio::io_context io_context_;
};

int main() {
  try {
    FdFileFinder file_finder{};
    auto files = file_finder.GetFiles();
    for (auto& file : files) {
      std::println("{}", file);
    }
  } catch (std::exception& e) {
    std::print("Error: {}\n", e.what());
  }
  return 0;
}
